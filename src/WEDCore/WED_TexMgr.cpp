/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#define NEW_TEX_LOAD_STRATEGY 1

#include "WED_TexMgr.h"
#include "BitmapUtils.h"
#include "MemFileUtils.h"
#include "TexUtils.h"
#include "WED_ResourceCache.h"
#include "WED_PackageMgr.h"
#include <chrono>
#include <cctype>
#include <cstdlib>

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

namespace {

static bool PerfOpenAnalysisEnabled()
{
	static const bool enabled = []() {
		const char * raw_value = getenv("WED_PERF_OPEN_ANALYSIS");
		if (raw_value == nullptr || raw_value[0] == '\0')
			return false;

		string value(raw_value);
		for (char& ch : value)
			ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));

		return value != "0" && value != "false" && value != "off" && value != "no";
	}();
	return enabled;
}

static bool EnvFlagEnabled(const char * name)
{
	const char * raw_value = getenv(name);
	if (raw_value == nullptr || raw_value[0] == '\0')
		return false;

	string value(raw_value);
	for (char& ch : value)
		ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));

	return value != "0" && value != "false" && value != "off" && value != "no";
}

static int EffectiveTextureFlags(int flags)
{
	return EnvFlagEnabled("WED_PERF_DISABLE_TEX_COMPRESS_OK") ? (flags & ~tex_Compress_Ok) : flags;
}

static bool IsDirectCompressedTexturePath(const string& path)
{
	string lowered(path);
	for (char& ch : lowered)
		ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));

	return lowered.size() >= 4 && (
		lowered.rfind(".dds") == lowered.size() - 4 ||
		lowered.rfind(".ktx2") == lowered.size() - 5);
}

}

WED_TexMgr::WED_TexMgr(const string& package) : mPackage(package)
{
}

WED_TexMgr::~WED_TexMgr()
{
	for(map<string,TexInfo *>::iterator t = mTexes.begin(); t != mTexes.end(); ++t)
	{
		GLuint id = t->second->tex_id;
		glDeleteTextures(1, &id);
		delete t->second;
	}
}

void 		WED_TexMgr::DropTexture(const char * path)
{
	TexMap::iterator i = mTexes.find(path);
	if (i != mTexes.end())
	{
		GLuint id = i->second->tex_id;
		glDeleteTextures(1, &id);
		delete i->second;
		mTexes.erase(i);
	}
}

TexRef		WED_TexMgr::LookupTexture(const char * path, bool is_absolute, int flags)
{
	TexMap::iterator i = mTexes.find(path);
	if (i == mTexes.end())
	{
		return LoadTexture(path, is_absolute,flags);
	}
	return i->second;
}

int			WED_TexMgr::GetTexID(TexRef ref)
{
	return ((TexInfo *) ref)->tex_id;
}

void		WED_TexMgr::GetTexInfo(
						TexRef	ref,
						int *	vis_x,
						int *	vis_y,
						int *	act_x,
						int *	act_y,
						int *	org_x,
						int *	org_y)
{
	TexInfo * i = (TexInfo *) ref;
	if (vis_x) *vis_x = i->vis_x;
	if (vis_y) *vis_y = i->vis_y;
	if (act_x) *act_x = i->act_x;
	if (act_y) *act_y = i->act_y;
	if (org_x) *org_x = i->org_x;
	if (org_y) *org_y = i->org_y;
}

WED_TexMgr::TexInfo *	WED_TexMgr::LoadTexture(const char * path, bool is_absolute, int flags)
{
	const bool perf_enabled = PerfOpenAnalysisEnabled();
	const auto perf_begin = perf_enabled ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point();
	string fpath(is_absolute ? path : gPackageMgr->ComputePath(mPackage, path));
	TexInfo * inf = NULL;
	const char * perf_mode = "source_prepare";

	GLuint tn;
	glGenTextures(1,&tn);
#if LOAD_DDS_DIRECT || LOAD_KTX2_DIRECT
	FILE * file = fopen(fpath.c_str(), "rb");
	char c[8];
	if (file && fread(c, 1, 8, file) == 8)
	{
		if (strncmp(c, "DDS ",4) == 0)
		{
			perf_mode = "direct_compressed";
			fseek(file, 0, SEEK_END);
			int fileLength = ftell(file);
			fseek(file, 0, SEEK_SET);
			char * buffer = new char[fileLength];
			if (buffer)
			{
				if (fread(buffer, 1, fileLength, file) == fileLength)
				{
					int siz_x, siz_y;
					if (LoadTextureFromDDS(buffer, buffer + fileLength, tn, flags, &siz_x, &siz_y))
					{
//						printf("Direct loading DDS %s\n", fpath.c_str());
						inf = new TexInfo;
						inf->tex_id = tn;
						inf->org_x = inf->vis_x = inf->act_x = siz_x;
						inf->org_y = inf->vis_y = inf->act_y = siz_y;
						mTexes[path] = inf;
					}
				}
				delete [] buffer;
			}
		}
#if LOAD_KTX2_DIRECT
		else if (strncmp(c, "\253KTX 20\273", 8) == 0)
		{
			perf_mode = "direct_compressed";
			fseek(file, 0, SEEK_END);
			int fileLength = ftell(file);
			fseek(file, 0, SEEK_SET);
			char* buffer = new char[fileLength];
			if (buffer)
			{
				if (fread(buffer, 1, fileLength, file) == fileLength)
				{
					int siz_x, siz_y;
					if (LoadTextureFromKTX2(buffer, buffer + fileLength, tn, flags, &siz_x, &siz_y))
					{
						//						printf("Direct loading KTX2 %s\n", fpath.c_str());
						inf = new TexInfo;
						inf->tex_id = tn;
						inf->org_x = inf->vis_x = inf->act_x = siz_x;
						inf->org_y = inf->vis_y = inf->act_y = siz_y;
						mTexes[path] = inf;
					}
				}
				delete[] buffer;
			}
		}
#endif
		fclose(file);
	}
	if(inf) return inf;

//	printf("Normal load %s\n", fpath.c_str());
#endif

#if NEW_TEX_LOAD_STRATEGY
	// auto-detection of file type, basic on file content, only
	{
		EnsureTextureUploadCapsInitializedOnDrawThread();
		PreparedTextureImage prepared;
		CompressedTextureImage compressed;
		PreparedTextureUploadPerf upload_perf;
		WED_ResourceCachePreparedTextureLoadPerf cache_perf;
		int siz_x = 0;
		int siz_y = 0;
		float s = 1.0f;
		float t = 1.0f;
		bool loaded = false;
		const int effective_flags = EffectiveTextureFlags(flags);
		const bool allow_compressed_cache = !IsDirectCompressedTexturePath(fpath) && ((effective_flags & tex_Compress_Ok) != 0);
		auto maybe_store_compressed_cache = [&]() {
			if (!allow_compressed_cache || upload_perf.compress_ok == 0)
				return;

			CompressedTextureImage captured;
			if (CaptureCompressedTextureFromBoundTexture(prepared, &captured))
				WED_ResourceCache::Get().StoreCompressedTexture(fpath, flags, captured);
			DestroyCompressedTextureImage(&captured);
		};

		if (allow_compressed_cache && WED_ResourceCache::Get().LoadCompressedTexture(fpath, flags, compressed, &cache_perf))
		{
			loaded = LoadTextureFromCompressedImage(compressed, tn, flags, &upload_perf);
			if (loaded)
			{
				perf_mode = "compressed_cache_hit";
				siz_x = compressed.act_x;
				siz_y = compressed.act_y;
				s = compressed.act_x > 0 ? static_cast<float>(compressed.vis_x) / static_cast<float>(compressed.act_x) : 1.0f;
				t = compressed.act_y > 0 ? static_cast<float>(compressed.vis_y) / static_cast<float>(compressed.act_y) : 1.0f;
				if (perf_enabled)
				{
					LOG_MSG(
						"I/Perf CompressedTextureHot path=%s compress_ok=%d cache_lookup=%.3lf s cache_read=%.3lf s cache_deserialize=%.3lf s cache_total=%.3lf s upload_gl=%.3lf s upload_configure=%.3lf s upload_total=%.3lf s bytes=%d levels=%d size=%dx%d internal_format=0x%X\n",
						fpath.c_str(),
						upload_perf.compress_ok,
						cache_perf.lookup_seconds,
						cache_perf.read_seconds,
						cache_perf.deserialize_seconds,
						cache_perf.total_seconds,
						upload_perf.upload_seconds,
						upload_perf.configure_seconds,
						upload_perf.total_seconds,
						upload_perf.data_size,
						upload_perf.level_count,
						upload_perf.width,
						upload_perf.height,
						compressed.internal_format);
				}
			}
			DestroyCompressedTextureImage(&compressed);
		}

		if (!loaded)
		{
			if (!IsDirectCompressedTexturePath(fpath) && WED_ResourceCache::Get().LoadPreparedTexture(fpath, flags, prepared, &cache_perf))
			{
				perf_mode = "prepared_cache_hit";
				loaded = LoadTextureFromPreparedImage(prepared, tn, flags, &upload_perf);
				if (loaded)
				{
					siz_x = prepared.act_x;
					siz_y = prepared.act_y;
					s = prepared.act_x > 0 ? static_cast<float>(prepared.vis_x) / static_cast<float>(prepared.act_x) : 1.0f;
					t = prepared.act_y > 0 ? static_cast<float>(prepared.vis_y) / static_cast<float>(prepared.act_y) : 1.0f;
					maybe_store_compressed_cache();
				}
				if (perf_enabled)
				{
					LOG_MSG(
						"I/Perf PreparedTextureHot path=%s compress_ok=%d cache_lookup=%.3lf s cache_read=%.3lf s cache_deserialize=%.3lf s cache_total=%.3lf s upload_convert=%.3lf s upload_gl=%.3lf s upload_configure=%.3lf s upload_total=%.3lf s bytes=%d levels=%d size=%dx%d channels=%d\n",
						fpath.c_str(),
						upload_perf.compress_ok,
						cache_perf.lookup_seconds,
						cache_perf.read_seconds,
						cache_perf.deserialize_seconds,
						cache_perf.total_seconds,
						upload_perf.convert_seconds,
						upload_perf.upload_seconds,
						upload_perf.configure_seconds,
						upload_perf.total_seconds,
						upload_perf.data_size,
						upload_perf.level_count,
						upload_perf.width,
						upload_perf.height,
						upload_perf.channels);
				}
			}

			if (!loaded)
			{
				ImageInfo im = { 0 };
				if (LoadBitmapFromAnyFile(fpath.c_str(), &im) == 0)
				{
					if (PrepareTextureImageForUpload(im, flags, &prepared))
					{
						loaded = LoadTextureFromPreparedImage(prepared, tn, flags, &upload_perf);
						if (loaded)
						{
							siz_x = prepared.act_x;
							siz_y = prepared.act_y;
							s = prepared.act_x > 0 ? static_cast<float>(prepared.vis_x) / static_cast<float>(prepared.act_x) : 1.0f;
							t = prepared.act_y > 0 ? static_cast<float>(prepared.vis_y) / static_cast<float>(prepared.act_y) : 1.0f;
							WED_ResourceCache::Get().StorePreparedTexture(fpath, flags, prepared);
							maybe_store_compressed_cache();
						}
						if (perf_enabled)
						{
							LOG_MSG(
								"I/Perf PreparedTextureSource path=%s compress_ok=%d upload_convert=%.3lf s upload_gl=%.3lf s upload_configure=%.3lf s upload_total=%.3lf s bytes=%d levels=%d size=%dx%d channels=%d\n",
								fpath.c_str(),
								upload_perf.compress_ok,
								upload_perf.convert_seconds,
								upload_perf.upload_seconds,
								upload_perf.configure_seconds,
								upload_perf.total_seconds,
								upload_perf.data_size,
								upload_perf.level_count,
								upload_perf.width,
								upload_perf.height,
								upload_perf.channels);
						}
					}
					else
					{
						perf_mode = "source_direct_fallback";
						loaded = LoadTextureFromImage(im, tn, flags, &siz_x, &siz_y, &s, &t);
					}

					if (im.data != nullptr)
						DestroyBitmap(&im);
				}
			}
		}
		if (loaded)
#else
	// loading based on file name suffix. With this method we preserve awareness of original image size.
	// But with openGL 3.0 as new minimum requirement - all GPU's have to support non-power-2 textures,
	// and only Orthophoto export would be affected. So thats unlikely to be a loss, ever.
	ImageInfo	im;
	if(MakeSupportedType(fpath.c_str(), &im) == 0)
	{
		int siz_x, siz_y;
		float s,t;
		if (LoadTextureFromImage(im, tn, flags, &siz_x, &siz_y, &s,&t))
#endif
		{
			inf = new TexInfo;
			inf->tex_id = tn;
#if NEW_TEX_LOAD_STRATEGY
			inf->org_x = prepared.org_x > 0 ? prepared.org_x : siz_x;
			inf->org_y = prepared.org_y > 0 ? prepared.org_y : siz_y;
#else
			inf->org_x = im.width;
			inf->org_y = im.height;
#endif
			inf->act_x = siz_x;
			inf->act_y = siz_y;
			inf->vis_x = (float) siz_x * s;
			inf->vis_y = (float) siz_y * t;
			mTexes[path] = inf;
		}
		DestroyPreparedTextureImage(&prepared);
#if !NEW_TEX_LOAD_STRATEGY
		DestroyBitmap(&im);
#endif
	}
	if (perf_enabled)
	{
		const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - perf_begin).count();
		LOG_MSG("I/Perf LoadTexture mode=%s path=%s elapsed=%.3lf s\n", perf_mode, fpath.c_str(), elapsed);
	}
	return inf;
}
