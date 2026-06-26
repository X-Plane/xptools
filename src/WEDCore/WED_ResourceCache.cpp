/*
 * Copyright (c) 2026
 */

#include "WED_ResourceCache.h"

#include "FileUtils.h"
#include "GUI_Prefs.h"
#include "MemFileUtils.h"
#include "PlatformUtils.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

#if IBM
#include <windows.h>
#else
#include <sys/statvfs.h>
#endif

extern "C" {
#include "sqlite3.h"
}

namespace {

static const int kObjMetaSchemaVersion = 1;
static const int kObjGeomSchemaVersion = 1;
static const int kTexPreparedSchemaVersion = 1;
static const int kTexCompressedSchemaVersion = 1;
static const int kNavaidIndexSchemaVersion = 1;
static const int kCacheLayoutVersion = 3;
static const unsigned int kPackMagic = 0x32524357;  // WCR2
static const long long kSegmentSizeBytes = 1LL << 30;
static const long long kConfiguredHardCapBytes = 16LL << 30;
static const long long kPruneSlackBytes = 2LL << 30;
static const long long kMinFreeReserveBytes = 4LL << 30;
static const long long kMaxFreeReserveBytes = 16LL << 30;
static const char * kPerformancePrefSection = "performance";
static const char * kPerformanceCacheArtifactKey = "cache_artifact";

struct PackEntryHeader {
	unsigned int magic;
	unsigned int kind;
	unsigned long long payload_size;
	unsigned int reserved;
};

struct BlobWriter {
	std::vector<unsigned char> data;

	template <typename T>
	void WritePod(const T& value)
	{
		const unsigned char * begin = reinterpret_cast<const unsigned char *>(&value);
		data.insert(data.end(), begin, begin + sizeof(T));
	}

	void WriteBytes(const void * src, size_t len)
	{
		const unsigned char * begin = reinterpret_cast<const unsigned char *>(src);
		data.insert(data.end(), begin, begin + len);
	}
};

struct BlobReader {
	const unsigned char * cur;
	const unsigned char * end;

	BlobReader(const unsigned char * in_begin, const unsigned char * in_end) : cur(in_begin), end(in_end) {}

	template <typename T>
	bool ReadPod(T& value)
	{
		if (static_cast<size_t>(end - cur) < sizeof(T))
			return false;
		memcpy(&value, cur, sizeof(T));
		cur += sizeof(T);
		return true;
	}

	bool ReadBytes(void * dst, size_t len)
	{
		if (static_cast<size_t>(end - cur) < len)
			return false;
		memcpy(dst, cur, len);
		cur += len;
		return true;
	}

	bool Done() const
	{
		return cur == end;
	}
};

static bool EnvFlagEnabled(const char * name)
{
	const char * raw = getenv(name);
	if (raw == nullptr || raw[0] == '\0')
		return false;

	std::string lowered(raw);
	std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](char ch) {
		return static_cast<char>(tolower(static_cast<unsigned char>(ch)));
	});
	return lowered != "0" && lowered != "false" && lowered != "off" && lowered != "no";
}

static bool EnvFlagEnabledOrDefault(const char * name, bool default_value)
{
	const char * raw = getenv(name);
	if (raw == nullptr || raw[0] == '\0')
		return default_value;
	return EnvFlagEnabled(name);
}

static std::string NormalizePathForKey(const std::string& path)
{
	std::string normalized(path);
	for (char& ch : normalized)
	{
#if IBM
		if (ch == '/')
			ch = '\\';
		ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
#else
		if (ch == '\\')
			ch = '/';
#endif
	}
	return normalized;
}

static unsigned long long Fnv1a64(const std::string& value)
{
	unsigned long long hash = 1469598103934665603ULL;
	for (unsigned char ch : value)
	{
		hash ^= static_cast<unsigned long long>(ch);
		hash *= 1099511628211ULL;
	}
	return hash;
}

static std::string HexU64(unsigned long long value)
{
	char buf[17] = { 0 };
	snprintf(buf, sizeof(buf), "%016llx", value);
	return std::string(buf);
}

static long long NowUnixSeconds()
{
	return static_cast<long long>(time(nullptr));
}

static bool ReadArtifactCacheEnabledPref()
{
	return atoi(GUI_GetPrefString(kPerformancePrefSection, kPerformanceCacheArtifactKey, "1")) != 0;
}

static std::string MakeSegmentFileName(long long sequence)
{
	char buf[64] = { 0 };
	snprintf(buf, sizeof(buf), "pack-%06lld.dat", sequence);
	return std::string(buf);
}

static bool QueryVolumeSpace(const std::string& path, unsigned long long& out_free_bytes, unsigned long long& out_total_bytes)
{
#if IBM
	ULARGE_INTEGER free_bytes = { 0 };
	ULARGE_INTEGER total_bytes = { 0 };
	ULARGE_INTEGER total_free_bytes = { 0 };
	if (!GetDiskFreeSpaceExA(path.c_str(), &free_bytes, &total_bytes, &total_free_bytes))
		return false;
	out_free_bytes = free_bytes.QuadPart;
	out_total_bytes = total_bytes.QuadPart;
	return true;
#else
	struct statvfs fs = { 0 };
	if (statvfs(path.c_str(), &fs) != 0)
		return false;
	out_free_bytes = static_cast<unsigned long long>(fs.f_bavail) * static_cast<unsigned long long>(fs.f_frsize);
	out_total_bytes = static_cast<unsigned long long>(fs.f_blocks) * static_cast<unsigned long long>(fs.f_frsize);
	return true;
#endif
}

static void WriteString(BlobWriter& writer, const std::string& value)
{
	unsigned int len = static_cast<unsigned int>(value.size());
	writer.WritePod(len);
	if (len)
		writer.WriteBytes(value.data(), len);
}

static bool ReadString(BlobReader& reader, std::string& out_value)
{
	unsigned int len = 0;
	if (!reader.ReadPod(len))
		return false;
	out_value.resize(len);
	return len == 0 || reader.ReadBytes(&out_value[0], len);
}

static void WriteFloatArray(BlobWriter& writer, const float * values, size_t count)
{
	for (size_t i = 0; i < count; ++i)
		writer.WritePod(values[i]);
}

static bool ReadFloatArray(BlobReader& reader, float * values, size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		if (!reader.ReadPod(values[i]))
			return false;
	}
	return true;
}

static void WriteDoubleArray(BlobWriter& writer, const double * values, size_t count)
{
	for (size_t i = 0; i < count; ++i)
		writer.WritePod(values[i]);
}

static bool ReadDoubleArray(BlobReader& reader, double * values, size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		if (!reader.ReadPod(values[i]))
			return false;
	}
	return true;
}

static void WriteIntVector(BlobWriter& writer, const std::vector<int>& values)
{
	unsigned int count = static_cast<unsigned int>(values.size());
	writer.WritePod(count);
	if (count)
		writer.WriteBytes(values.data(), count * sizeof(int));
}

static bool ReadIntVector(BlobReader& reader, std::vector<int>& values)
{
	unsigned int count = 0;
	if (!reader.ReadPod(count))
		return false;
	values.resize(count);
	return count == 0 || reader.ReadBytes(values.data(), count * sizeof(int));
}

static void WriteObjPointPool(BlobWriter& writer, const ObjPointPool& pool, int depth)
{
	unsigned int count = static_cast<unsigned int>(pool.count());
	writer.WritePod(depth);
	writer.WritePod(count);
	for (unsigned int i = 0; i < count; ++i)
		writer.WriteBytes(pool.get(static_cast<int>(i)), static_cast<size_t>(depth) * sizeof(float));
}

static bool ReadObjPointPool(BlobReader& reader, ObjPointPool& pool)
{
	int depth = 0;
	unsigned int count = 0;
	if (!reader.ReadPod(depth) || !reader.ReadPod(count))
		return false;
	if (depth <= 0)
		return false;
	pool.clear(depth);
	pool.resize(static_cast<int>(count));
	for (unsigned int i = 0; i < count; ++i)
	{
		if (!reader.ReadBytes(pool.get(static_cast<int>(i)), static_cast<size_t>(depth) * sizeof(float)))
			return false;
	}
	return true;
}

static void WriteObjKeyVector(BlobWriter& writer, const std::vector<XObjKey>& values)
{
	unsigned int count = static_cast<unsigned int>(values.size());
	writer.WritePod(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		writer.WritePod(values[i].key);
		WriteFloatArray(writer, values[i].v, 3);
	}
}

static bool ReadObjKeyVector(BlobReader& reader, std::vector<XObjKey>& values)
{
	unsigned int count = 0;
	if (!reader.ReadPod(count))
		return false;
	values.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		if (!reader.ReadPod(values[i].key) || !ReadFloatArray(reader, values[i].v, 3))
			return false;
	}
	return true;
}

static void WriteObjDetentVector(BlobWriter& writer, const std::vector<XObjDetentRange>& values)
{
	unsigned int count = static_cast<unsigned int>(values.size());
	writer.WritePod(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		writer.WritePod(values[i].lo);
		writer.WritePod(values[i].hi);
		writer.WritePod(values[i].height);
	}
}

static bool ReadObjDetentVector(BlobReader& reader, std::vector<XObjDetentRange>& values)
{
	unsigned int count = 0;
	if (!reader.ReadPod(count))
		return false;
	values.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		if (!reader.ReadPod(values[i].lo) ||
			!reader.ReadPod(values[i].hi) ||
			!reader.ReadPod(values[i].height))
			return false;
	}
	return true;
}

static void WriteObjAnimationVector(BlobWriter& writer, const std::vector<XObjAnim8>& values)
{
	unsigned int count = static_cast<unsigned int>(values.size());
	writer.WritePod(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		writer.WritePod(values[i].cmd);
		WriteString(writer, values[i].dataref);
		WriteFloatArray(writer, values[i].axis, 3);
		writer.WritePod(values[i].loop);
		WriteObjKeyVector(writer, values[i].keyframes);
	}
}

static bool ReadObjAnimationVector(BlobReader& reader, std::vector<XObjAnim8>& values)
{
	unsigned int count = 0;
	if (!reader.ReadPod(count))
		return false;
	values.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		if (!reader.ReadPod(values[i].cmd) ||
			!ReadString(reader, values[i].dataref) ||
			!ReadFloatArray(reader, values[i].axis, 3) ||
			!reader.ReadPod(values[i].loop) ||
			!ReadObjKeyVector(reader, values[i].keyframes))
			return false;
	}
	return true;
}

static void WriteObjManipVector(BlobWriter& writer, const std::vector<XObjManip8>& values)
{
	unsigned int count = static_cast<unsigned int>(values.size());
	writer.WritePod(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		WriteString(writer, values[i].dataref1);
		WriteString(writer, values[i].dataref2);
		WriteFloatArray(writer, values[i].centroid, 3);
		WriteFloatArray(writer, values[i].axis, 3);
		writer.WritePod(values[i].angle_min);
		writer.WritePod(values[i].angle_max);
		writer.WritePod(values[i].lift);
		writer.WritePod(values[i].v1_min);
		writer.WritePod(values[i].v1_max);
		writer.WritePod(values[i].v2_min);
		writer.WritePod(values[i].v2_max);
		WriteString(writer, values[i].cursor);
		WriteString(writer, values[i].tooltip);
		writer.WritePod(values[i].mouse_wheel_delta);
		WriteObjKeyVector(writer, values[i].rotation_key_frames);
		WriteObjDetentVector(writer, values[i].detents);
	}
}

static bool ReadObjManipVector(BlobReader& reader, std::vector<XObjManip8>& values)
{
	unsigned int count = 0;
	if (!reader.ReadPod(count))
		return false;
	values.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		if (!ReadString(reader, values[i].dataref1) ||
			!ReadString(reader, values[i].dataref2) ||
			!ReadFloatArray(reader, values[i].centroid, 3) ||
			!ReadFloatArray(reader, values[i].axis, 3) ||
			!reader.ReadPod(values[i].angle_min) ||
			!reader.ReadPod(values[i].angle_max) ||
			!reader.ReadPod(values[i].lift) ||
			!reader.ReadPod(values[i].v1_min) ||
			!reader.ReadPod(values[i].v1_max) ||
			!reader.ReadPod(values[i].v2_min) ||
			!reader.ReadPod(values[i].v2_max) ||
			!ReadString(reader, values[i].cursor) ||
			!ReadString(reader, values[i].tooltip) ||
			!reader.ReadPod(values[i].mouse_wheel_delta) ||
			!ReadObjKeyVector(reader, values[i].rotation_key_frames) ||
			!ReadObjDetentVector(reader, values[i].detents))
			return false;
	}
	return true;
}

static void WriteObjEmitterVector(BlobWriter& writer, const std::vector<XObjEmitter8>& values)
{
	unsigned int count = static_cast<unsigned int>(values.size());
	writer.WritePod(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		WriteString(writer, values[i].name);
		WriteString(writer, values[i].dataref);
		writer.WritePod(values[i].x);
		writer.WritePod(values[i].y);
		writer.WritePod(values[i].z);
		writer.WritePod(values[i].psi);
		writer.WritePod(values[i].the);
		writer.WritePod(values[i].phi);
		writer.WritePod(values[i].v_min);
		writer.WritePod(values[i].v_max);
	}
}

static bool ReadObjEmitterVector(BlobReader& reader, std::vector<XObjEmitter8>& values)
{
	unsigned int count = 0;
	if (!reader.ReadPod(count))
		return false;
	values.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		if (!ReadString(reader, values[i].name) ||
			!ReadString(reader, values[i].dataref) ||
			!reader.ReadPod(values[i].x) ||
			!reader.ReadPod(values[i].y) ||
			!reader.ReadPod(values[i].z) ||
			!reader.ReadPod(values[i].psi) ||
			!reader.ReadPod(values[i].the) ||
			!reader.ReadPod(values[i].phi) ||
			!reader.ReadPod(values[i].v_min) ||
			!reader.ReadPod(values[i].v_max))
			return false;
	}
	return true;
}

static void WriteObjCmdVector(BlobWriter& writer, const std::vector<XObjCmd8>& values)
{
	unsigned int count = static_cast<unsigned int>(values.size());
	writer.WritePod(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		writer.WritePod(values[i].cmd);
		WriteFloatArray(writer, values[i].params, 12);
		WriteString(writer, values[i].name);
		writer.WritePod(values[i].idx_offset);
		writer.WritePod(values[i].idx_count);
	}
}

static bool ReadObjCmdVector(BlobReader& reader, std::vector<XObjCmd8>& values)
{
	unsigned int count = 0;
	if (!reader.ReadPod(count))
		return false;
	values.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		if (!reader.ReadPod(values[i].cmd) ||
			!ReadFloatArray(reader, values[i].params, 12) ||
			!ReadString(reader, values[i].name) ||
			!reader.ReadPod(values[i].idx_offset) ||
			!reader.ReadPod(values[i].idx_count))
			return false;
	}
	return true;
}

static void WriteObjLodVector(BlobWriter& writer, const std::vector<XObjLOD8>& values)
{
	unsigned int count = static_cast<unsigned int>(values.size());
	writer.WritePod(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		writer.WritePod(values[i].lod_near);
		writer.WritePod(values[i].lod_far);
		WriteObjCmdVector(writer, values[i].cmds);
	}
}

static bool ReadObjLodVector(BlobReader& reader, std::vector<XObjLOD8>& values)
{
	unsigned int count = 0;
	if (!reader.ReadPod(count))
		return false;
	values.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		if (!reader.ReadPod(values[i].lod_near) ||
			!reader.ReadPod(values[i].lod_far) ||
			!ReadObjCmdVector(reader, values[i].cmds))
			return false;
	}
	return true;
}

static void WriteObjRegionVector(BlobWriter& writer, const std::vector<XObjPanelRegion8>& values)
{
	unsigned int count = static_cast<unsigned int>(values.size());
	writer.WritePod(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		writer.WritePod(values[i].left);
		writer.WritePod(values[i].bottom);
		writer.WritePod(values[i].right);
		writer.WritePod(values[i].top);
	}
}

static bool ReadObjRegionVector(BlobReader& reader, std::vector<XObjPanelRegion8>& values)
{
	unsigned int count = 0;
	if (!reader.ReadPod(count))
		return false;
	values.resize(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		if (!reader.ReadPod(values[i].left) ||
			!reader.ReadPod(values[i].bottom) ||
			!reader.ReadPod(values[i].right) ||
			!reader.ReadPod(values[i].top))
			return false;
	}
	return true;
}

static WED_ResourceCacheObjMeta ExtractObjMeta(const XObj8& obj)
{
	WED_ResourceCacheObjMeta meta;
	meta.fixed_heading = obj.fixed_heading;
	meta.viewpoint_height = obj.viewpoint_height;
	for (int i = 0; i < 3; ++i)
	{
		meta.xyz_min[i] = obj.xyz_min[i];
		meta.xyz_max[i] = obj.xyz_max[i];
	}
	return meta;
}

static std::vector<unsigned char> SerializeObjMeta(const WED_ResourceCacheObjMeta& meta)
{
	BlobWriter writer;
	writer.WritePod(meta.fixed_heading);
	writer.WritePod(meta.viewpoint_height);
	WriteFloatArray(writer, meta.xyz_min, 3);
	WriteFloatArray(writer, meta.xyz_max, 3);
	return writer.data;
}

static bool DeserializeObjMeta(const std::vector<unsigned char>& bytes, WED_ResourceCacheObjMeta& out_meta)
{
	BlobReader reader(bytes.data(), bytes.data() + bytes.size());
	return reader.ReadPod(out_meta.fixed_heading) &&
		reader.ReadPod(out_meta.viewpoint_height) &&
		ReadFloatArray(reader, out_meta.xyz_min, 3) &&
		ReadFloatArray(reader, out_meta.xyz_max, 3) &&
		reader.Done();
}

static std::vector<unsigned char> SerializeObjGeom(const XObj8& obj)
{
	BlobWriter writer;
	WriteString(writer, obj.texture);
	WriteString(writer, obj.texture_normal_map);
	WriteString(writer, obj.texture_lit);
	WriteString(writer, obj.decal_lib);
	WriteString(writer, obj.texture_draped);
	writer.WritePod(obj.use_metalness);
	writer.WritePod(obj.glass_blending);
	WriteString(writer, obj.particle_system);
	WriteObjRegionVector(writer, obj.regions);
	WriteIntVector(writer, obj.indices);
	WriteObjPointPool(writer, obj.geo_tri, 8);
	WriteObjPointPool(writer, obj.geo_lines, 6);
	WriteObjPointPool(writer, obj.geo_lights, 6);
	WriteObjAnimationVector(writer, obj.animation);
	WriteObjManipVector(writer, obj.manips);
	WriteObjEmitterVector(writer, obj.emitters);
	WriteObjLodVector(writer, obj.lods);
	WriteFloatArray(writer, obj.xyz_min, 3);
	WriteFloatArray(writer, obj.xyz_max, 3);
	WriteDoubleArray(writer, obj.loadCenter_latlon, 2);
	writer.WritePod(obj.loadCenter_texSize);
	writer.WritePod(obj.loadCenter_size);
	writer.WritePod(obj.fixed_heading);
	writer.WritePod(obj.viewpoint_height);
	WriteString(writer, obj.description);
	return writer.data;
}

static bool DeserializeObjGeom(const std::vector<unsigned char>& bytes, XObj8& out_obj)
{
	BlobReader reader(bytes.data(), bytes.data() + bytes.size());
	if (!ReadString(reader, out_obj.texture) ||
		!ReadString(reader, out_obj.texture_normal_map) ||
		!ReadString(reader, out_obj.texture_lit) ||
		!ReadString(reader, out_obj.decal_lib) ||
		!ReadString(reader, out_obj.texture_draped) ||
		!reader.ReadPod(out_obj.use_metalness) ||
		!reader.ReadPod(out_obj.glass_blending) ||
		!ReadString(reader, out_obj.particle_system) ||
		!ReadObjRegionVector(reader, out_obj.regions) ||
		!ReadIntVector(reader, out_obj.indices) ||
		!ReadObjPointPool(reader, out_obj.geo_tri) ||
		!ReadObjPointPool(reader, out_obj.geo_lines) ||
		!ReadObjPointPool(reader, out_obj.geo_lights) ||
		!ReadObjAnimationVector(reader, out_obj.animation) ||
		!ReadObjManipVector(reader, out_obj.manips) ||
		!ReadObjEmitterVector(reader, out_obj.emitters) ||
		!ReadObjLodVector(reader, out_obj.lods) ||
		!ReadFloatArray(reader, out_obj.xyz_min, 3) ||
		!ReadFloatArray(reader, out_obj.xyz_max, 3) ||
		!ReadDoubleArray(reader, out_obj.loadCenter_latlon, 2) ||
		!reader.ReadPod(out_obj.loadCenter_texSize) ||
		!reader.ReadPod(out_obj.loadCenter_size) ||
		!reader.ReadPod(out_obj.fixed_heading) ||
		!reader.ReadPod(out_obj.viewpoint_height) ||
		!ReadString(reader, out_obj.description))
		return false;

	return reader.Done();
}

static std::vector<unsigned char> SerializePreparedTexture(const PreparedTextureImage& image)
{
	BlobWriter writer;
	writer.WritePod(image.data_size);
	writer.WritePod(image.width);
	writer.WritePod(image.height);
	writer.WritePod(image.channels);
	writer.WritePod(image.level_count);
	writer.WritePod(image.org_x);
	writer.WritePod(image.org_y);
	writer.WritePod(image.act_x);
	writer.WritePod(image.act_y);
	writer.WritePod(image.vis_x);
	writer.WritePod(image.vis_y);
	if (image.data_size > 0 && image.data != nullptr)
		writer.WriteBytes(image.data, image.data_size);
	return writer.data;
}

static bool DeserializePreparedTexture(const std::vector<unsigned char>& bytes, PreparedTextureImage& out_image)
{
	DestroyPreparedTextureImage(&out_image);
	BlobReader reader(bytes.data(), bytes.data() + bytes.size());
	if (!reader.ReadPod(out_image.data_size) ||
		!reader.ReadPod(out_image.width) ||
		!reader.ReadPod(out_image.height) ||
		!reader.ReadPod(out_image.channels) ||
		!reader.ReadPod(out_image.level_count) ||
		!reader.ReadPod(out_image.org_x) ||
		!reader.ReadPod(out_image.org_y) ||
		!reader.ReadPod(out_image.act_x) ||
		!reader.ReadPod(out_image.act_y) ||
		!reader.ReadPod(out_image.vis_x) ||
		!reader.ReadPod(out_image.vis_y))
		return false;

	if (out_image.data_size < 0)
		return false;

	if (out_image.data_size > 0)
	{
		out_image.data = reinterpret_cast<unsigned char *>(malloc(out_image.data_size));
		if (out_image.data == nullptr)
			return false;
		if (!reader.ReadBytes(out_image.data, out_image.data_size))
		{
			DestroyPreparedTextureImage(&out_image);
			return false;
		}
	}

	return reader.Done();
}

static std::vector<unsigned char> SerializeCompressedTexture(const CompressedTextureImage& image)
{
	BlobWriter writer;
	writer.WritePod(image.data_size);
	writer.WritePod(image.internal_format);
	writer.WritePod(image.width);
	writer.WritePod(image.height);
	writer.WritePod(image.level_count);
	writer.WritePod(image.org_x);
	writer.WritePod(image.org_y);
	writer.WritePod(image.act_x);
	writer.WritePod(image.act_y);
	writer.WritePod(image.vis_x);
	writer.WritePod(image.vis_y);
	WriteIntVector(writer, image.level_sizes);
	if (image.data_size > 0 && image.data != nullptr)
		writer.WriteBytes(image.data, image.data_size);
	return writer.data;
}

static bool DeserializeCompressedTexture(const std::vector<unsigned char>& bytes, CompressedTextureImage& out_image)
{
	DestroyCompressedTextureImage(&out_image);
	BlobReader reader(bytes.data(), bytes.data() + bytes.size());
	if (!reader.ReadPod(out_image.data_size) ||
		!reader.ReadPod(out_image.internal_format) ||
		!reader.ReadPod(out_image.width) ||
		!reader.ReadPod(out_image.height) ||
		!reader.ReadPod(out_image.level_count) ||
		!reader.ReadPod(out_image.org_x) ||
		!reader.ReadPod(out_image.org_y) ||
		!reader.ReadPod(out_image.act_x) ||
		!reader.ReadPod(out_image.act_y) ||
		!reader.ReadPod(out_image.vis_x) ||
		!reader.ReadPod(out_image.vis_y) ||
		!ReadIntVector(reader, out_image.level_sizes))
		return false;

	if (out_image.data_size <= 0 ||
		out_image.internal_format == 0 ||
		out_image.width <= 0 ||
		out_image.height <= 0 ||
		out_image.level_count <= 0 ||
		static_cast<int>(out_image.level_sizes.size()) != out_image.level_count)
	{
		DestroyCompressedTextureImage(&out_image);
		return false;
	}

	out_image.data = reinterpret_cast<unsigned char *>(malloc(out_image.data_size));
	if (out_image.data == nullptr)
		return false;
	if (!reader.ReadBytes(out_image.data, out_image.data_size))
	{
		DestroyCompressedTextureImage(&out_image);
		return false;
	}

	return reader.Done();
}

static bool ExecSql(sqlite3 * db, const char * sql)
{
	return sqlite3_exec(db, sql, nullptr, nullptr, nullptr) == SQLITE_OK;
}

static sqlite3_int64 FileOffsetEnd(FILE * file)
{
#if IBM
	_fseeki64(file, 0, SEEK_END);
	return _ftelli64(file);
#else
	fseeko(file, 0, SEEK_END);
	return ftello(file);
#endif
}

}

WED_ResourceCache& WED_ResourceCache::Get()
{
	static WED_ResourceCache cache;
	return cache;
}

WED_ResourceCache::WED_ResourceCache() :
	mEnabled(EnvFlagEnabledOrDefault("WED_RESOURCE_CACHE_V2", ReadArtifactCacheEnabledPref())),
	mInitAttempted(false),
	mOpenOk(false),
	mActivePackSize(0),
	mNextPackSequence(1),
	mDb(nullptr)
{
	const char * local_app_data = getenv("LOCALAPPDATA");
	if (local_app_data && local_app_data[0] != '\0')
		mRootPath = std::string(local_app_data) + DIR_STR "xptools" DIR_STR "resource-cache-v2";
	else
		mRootPath = GetCacheFolder() + DIR_STR "xptools" DIR_STR "resource-cache-v2";
	mDbPath = mRootPath + DIR_STR "cache.db";
	mPackDirPath = mRootPath + DIR_STR "packs";
}

WED_ResourceCache::~WED_ResourceCache()
{
	std::lock_guard<std::mutex> guard(mMutex);
	CloseLocked();
}

bool WED_ResourceCache::Enabled() const
{
	return mEnabled;
}

std::string WED_ResourceCache::GetRootPath() const
{
	return mRootPath;
}

void WED_ResourceCache::SetEnabled(bool enabled)
{
	std::lock_guard<std::mutex> guard(mMutex);
	mEnabled = enabled;
	CloseLocked();
	mInitAttempted = false;
	mActivePackName.clear();
	mActivePackSize = 0;
	mNextPackSequence = 1;
}

bool WED_ResourceCache::Clear()
{
	std::lock_guard<std::mutex> guard(mMutex);
	CloseLocked();
	mInitAttempted = false;
	mActivePackName.clear();
	mActivePackSize = 0;
	mNextPackSequence = 1;
	return ResetStorageLocked();
}

bool WED_ResourceCache::EnsureOpenLocked()
{
	if (!mEnabled)
		return false;
	if (mInitAttempted)
		return mOpenOk;

	mInitAttempted = true;
	if (FILE_make_dir_exist(mRootPath.c_str()) != 0)
		return false;
	if (FILE_make_dir_exist(mPackDirPath.c_str()) != 0)
		return false;

	if (sqlite3_open(mDbPath.c_str(), &mDb) != SQLITE_OK)
	{
		if (mDb)
			sqlite3_close(mDb);
		mDb = nullptr;
		return false;
	}

	ExecSql(mDb, "PRAGMA journal_mode=WAL;");
	ExecSql(mDb, "PRAGMA synchronous=NORMAL;");
	ExecSql(mDb, "PRAGMA temp_store=MEMORY;");
	if (!EnsureSchemaLocked())
	{
		CloseLocked();
		return false;
	}

	ExecSql(mDb, "UPDATE segments SET sealed = 1;");
	mActivePackName.clear();
	mActivePackSize = 0;

	const long long current_total = QueryTotalSegmentBytesLocked();
	const long long effective_cap = ComputeEffectiveHardCapBytesLocked(current_total);
	if (current_total > effective_cap)
	{
		const long long prune_target = ComputePruneTargetBytesLocked(effective_cap);
		while (QueryTotalSegmentBytesLocked() > prune_target)
		{
			SegmentRecord record;
			if (!FindOldestSealedPackLocked(record))
				break;
			if (!DeletePackLocked(record))
				break;
		}
	}

	mOpenOk = true;
	return true;
}

void WED_ResourceCache::CloseLocked()
{
	if (mDb != nullptr)
	{
		sqlite3_close(mDb);
		mDb = nullptr;
	}
	mOpenOk = false;
	mActivePackName.clear();
	mActivePackSize = 0;
}

bool WED_ResourceCache::EnsureSchemaLocked()
{
	if (!ExecSql(mDb,
		"CREATE TABLE IF NOT EXISTS meta ("
		"key TEXT PRIMARY KEY,"
		"value INTEGER NOT NULL"
		");"))
	{
		return false;
	}

	if (!ExecSql(mDb,
		"CREATE TABLE IF NOT EXISTS artifacts ("
		"cache_key TEXT PRIMARY KEY,"
		"kind INTEGER NOT NULL,"
		"schema_version INTEGER NOT NULL,"
		"flags INTEGER NOT NULL,"
		"source_path TEXT NOT NULL,"
		"source_size INTEGER NOT NULL,"
		"source_mtime INTEGER NOT NULL,"
		"pack_name TEXT NOT NULL,"
		"pack_offset INTEGER NOT NULL,"
		"pack_size INTEGER NOT NULL,"
		"created_at INTEGER NOT NULL,"
		"last_access INTEGER NOT NULL,"
		"hit_count INTEGER NOT NULL DEFAULT 0"
		");"))
	{
		return false;
	}

	if (!ExecSql(mDb,
		"CREATE TABLE IF NOT EXISTS segments ("
		"pack_name TEXT PRIMARY KEY,"
		"sequence INTEGER NOT NULL,"
		"created_at INTEGER NOT NULL,"
		"total_bytes INTEGER NOT NULL,"
		"sealed INTEGER NOT NULL"
		");"))
	{
		return false;
	}

	ExecSql(mDb, "CREATE INDEX IF NOT EXISTS idx_artifacts_pack_name ON artifacts(pack_name);");
	ExecSql(mDb, "CREATE INDEX IF NOT EXISTS idx_segments_sequence ON segments(sequence);");

	long long layout_version = 0;
	sqlite3_stmt * meta_stmt = nullptr;
	const bool has_meta_value =
		sqlite3_prepare_v2(mDb, "SELECT value FROM meta WHERE key = 'layout_version';", -1, &meta_stmt, nullptr) == SQLITE_OK &&
		sqlite3_step(meta_stmt) == SQLITE_ROW;
	if (has_meta_value)
		layout_version = sqlite3_column_int64(meta_stmt, 0);
	if (meta_stmt != nullptr)
		sqlite3_finalize(meta_stmt);

	if (!has_meta_value || layout_version != kCacheLayoutVersion)
	{
		CloseLocked();
		if (!ResetStorageLocked())
			return false;
		if (sqlite3_open(mDbPath.c_str(), &mDb) != SQLITE_OK)
		{
			if (mDb)
				sqlite3_close(mDb);
			mDb = nullptr;
			return false;
		}
		ExecSql(mDb, "PRAGMA journal_mode=WAL;");
		ExecSql(mDb, "PRAGMA synchronous=NORMAL;");
		ExecSql(mDb, "PRAGMA temp_store=MEMORY;");
		if (!ExecSql(mDb,
			"CREATE TABLE IF NOT EXISTS meta ("
			"key TEXT PRIMARY KEY,"
			"value INTEGER NOT NULL"
			");") ||
			!ExecSql(mDb,
			"CREATE TABLE IF NOT EXISTS artifacts ("
			"cache_key TEXT PRIMARY KEY,"
			"kind INTEGER NOT NULL,"
			"schema_version INTEGER NOT NULL,"
			"flags INTEGER NOT NULL,"
			"source_path TEXT NOT NULL,"
			"source_size INTEGER NOT NULL,"
			"source_mtime INTEGER NOT NULL,"
			"pack_name TEXT NOT NULL,"
			"pack_offset INTEGER NOT NULL,"
			"pack_size INTEGER NOT NULL,"
			"created_at INTEGER NOT NULL,"
			"last_access INTEGER NOT NULL,"
			"hit_count INTEGER NOT NULL DEFAULT 0"
			");") ||
			!ExecSql(mDb,
			"CREATE TABLE IF NOT EXISTS segments ("
			"pack_name TEXT PRIMARY KEY,"
			"sequence INTEGER NOT NULL,"
			"created_at INTEGER NOT NULL,"
			"total_bytes INTEGER NOT NULL,"
			"sealed INTEGER NOT NULL"
			");"))
		{
			return false;
		}
		ExecSql(mDb, "CREATE INDEX IF NOT EXISTS idx_artifacts_pack_name ON artifacts(pack_name);");
		ExecSql(mDb, "CREATE INDEX IF NOT EXISTS idx_segments_sequence ON segments(sequence);");
		sqlite3_stmt * insert_stmt = nullptr;
		if (sqlite3_prepare_v2(mDb, "INSERT OR REPLACE INTO meta (key, value) VALUES ('layout_version', ?1), ('next_pack_sequence', ?2);", -1, &insert_stmt, nullptr) == SQLITE_OK)
		{
			sqlite3_bind_int64(insert_stmt, 1, kCacheLayoutVersion);
			sqlite3_bind_int64(insert_stmt, 2, 1);
			sqlite3_step(insert_stmt);
			sqlite3_finalize(insert_stmt);
		}
		mNextPackSequence = 1;
		return true;
	}

	sqlite3_stmt * seq_stmt = nullptr;
	if (sqlite3_prepare_v2(mDb, "SELECT value FROM meta WHERE key = 'next_pack_sequence';", -1, &seq_stmt, nullptr) == SQLITE_OK &&
		sqlite3_step(seq_stmt) == SQLITE_ROW)
	{
		mNextPackSequence = sqlite3_column_int64(seq_stmt, 0);
	}
	else
	{
		mNextPackSequence = 1;
		sqlite3_stmt * insert_stmt = nullptr;
		if (sqlite3_prepare_v2(mDb, "INSERT OR REPLACE INTO meta (key, value) VALUES ('next_pack_sequence', ?1);", -1, &insert_stmt, nullptr) == SQLITE_OK)
		{
			sqlite3_bind_int64(insert_stmt, 1, mNextPackSequence);
			sqlite3_step(insert_stmt);
			sqlite3_finalize(insert_stmt);
		}
	}
	if (seq_stmt != nullptr)
		sqlite3_finalize(seq_stmt);
	return true;
}

bool WED_ResourceCache::ResetStorageLocked()
{
	if (FILE_exists(mRootPath.c_str()))
	{
		const string delete_path = mRootPath + DIR_STR;
		const int delete_rc = FILE_delete_dir_recursive(delete_path);
		if (delete_rc != 0)
			return false;
	}
	const int mk_root_rc = FILE_make_dir_exist(mRootPath.c_str());
	if (mk_root_rc != 0)
		return false;
	const int mk_pack_rc = FILE_make_dir_exist(mPackDirPath.c_str());
	if (mk_pack_rc != 0)
		return false;
	return true;
}

WED_ResourceCache::SourceFingerprint WED_ResourceCache::MakeFingerprint(const std::string& source_path) const
{
	SourceFingerprint fp;
	struct stat meta_data = { 0 };
	if (FILE_get_file_meta_data(source_path, meta_data) != 0)
		return fp;
	fp.normalized_path = NormalizePathForKey(source_path);
	fp.size = static_cast<long long>(meta_data.st_size);
	fp.mtime = static_cast<long long>(meta_data.st_mtime);
	fp.valid = true;
	return fp;
}

WED_ResourceCache::SourceFingerprint WED_ResourceCache::MakeVirtualFingerprint(const std::string& source_identity) const
{
	SourceFingerprint fp;
	fp.normalized_path = NormalizePathForKey(std::string("virtual:") + source_identity);
	fp.size = 0;
	fp.mtime = 0;
	fp.valid = !source_identity.empty();
	return fp;
}

std::string WED_ResourceCache::BuildCacheKey(ArtifactKind kind, const SourceFingerprint& fp, int flags, int schema_version) const
{
	std::string key_material = fp.normalized_path;
	key_material += "|";
	key_material += std::to_string(static_cast<int>(kind));
	key_material += "|";
	key_material += std::to_string(schema_version);
	key_material += "|";
	key_material += std::to_string(flags);
	key_material += "|";
	key_material += std::to_string(fp.size);
	key_material += "|";
	key_material += std::to_string(fp.mtime);
	return HexU64(Fnv1a64(key_material));
}

bool WED_ResourceCache::EnsureActivePackLocked()
{
	if (!mActivePackName.empty())
		return true;
	return CreateNewActivePackLocked();
}

bool WED_ResourceCache::CreateNewActivePackLocked()
{
	const std::string pack_name = MakeSegmentFileName(mNextPackSequence);
	sqlite3_stmt * stmt = nullptr;
	if (sqlite3_prepare_v2(
			mDb,
			"INSERT INTO segments (pack_name, sequence, created_at, total_bytes, sealed) VALUES (?1, ?2, ?3, 0, 0);",
			-1,
			&stmt,
			nullptr) != SQLITE_OK)
	{
		return false;
	}

	const long long now = NowUnixSeconds();
	sqlite3_bind_text(stmt, 1, pack_name.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int64(stmt, 2, mNextPackSequence);
	sqlite3_bind_int64(stmt, 3, now);
	const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
	sqlite3_finalize(stmt);
	if (!ok)
		return false;

	sqlite3_stmt * meta_stmt = nullptr;
	if (sqlite3_prepare_v2(mDb, "INSERT OR REPLACE INTO meta (key, value) VALUES ('next_pack_sequence', ?1);", -1, &meta_stmt, nullptr) == SQLITE_OK)
	{
		sqlite3_bind_int64(meta_stmt, 1, mNextPackSequence + 1);
		sqlite3_step(meta_stmt);
		sqlite3_finalize(meta_stmt);
	}
	++mNextPackSequence;

	const std::string pack_path = mPackDirPath + DIR_STR + pack_name;
	FILE * file = fopen(pack_path.c_str(), "ab");
	if (file == nullptr)
		return false;
	fclose(file);

	mActivePackName = pack_name;
	mActivePackSize = 0;
	return true;
}

void WED_ResourceCache::SealActivePackLocked()
{
	if (mActivePackName.empty())
		return;

	sqlite3_stmt * stmt = nullptr;
	if (sqlite3_prepare_v2(mDb, "UPDATE segments SET sealed = 1 WHERE pack_name = ?1;", -1, &stmt, nullptr) == SQLITE_OK)
	{
		sqlite3_bind_text(stmt, 1, mActivePackName.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
	mActivePackName.clear();
	mActivePackSize = 0;
}

bool WED_ResourceCache::FindOldestSealedPackLocked(SegmentRecord& out_record)
{
	out_record = SegmentRecord();
	sqlite3_stmt * stmt = nullptr;
	if (sqlite3_prepare_v2(
			mDb,
			"SELECT pack_name, sequence, total_bytes, sealed FROM segments WHERE sealed = 1 ORDER BY sequence ASC LIMIT 1;",
			-1,
			&stmt,
			nullptr) != SQLITE_OK)
	{
		return false;
	}

	if (sqlite3_step(stmt) != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return false;
	}

	const unsigned char * pack_name_text = sqlite3_column_text(stmt, 0);
	out_record.pack_name = pack_name_text ? reinterpret_cast<const char *>(pack_name_text) : "";
	out_record.sequence = sqlite3_column_int64(stmt, 1);
	out_record.total_bytes = sqlite3_column_int64(stmt, 2);
	out_record.sealed = sqlite3_column_int(stmt, 3) != 0;
	out_record.valid = !out_record.pack_name.empty();
	sqlite3_finalize(stmt);
	return out_record.valid;
}

bool WED_ResourceCache::DeletePackLocked(const SegmentRecord& record)
{
	if (!record.valid || record.pack_name.empty())
		return false;

	const std::string pack_path = mPackDirPath + DIR_STR + record.pack_name;
	if (FILE_exists(pack_path.c_str()) && FILE_delete_file(pack_path.c_str(), false) != 0)
		return false;

	ExecSql(mDb, "BEGIN IMMEDIATE TRANSACTION;");

	bool ok = true;
	sqlite3_stmt * delete_artifacts = nullptr;
	if (sqlite3_prepare_v2(mDb, "DELETE FROM artifacts WHERE pack_name = ?1;", -1, &delete_artifacts, nullptr) == SQLITE_OK)
	{
		sqlite3_bind_text(delete_artifacts, 1, record.pack_name.c_str(), -1, SQLITE_TRANSIENT);
		ok = sqlite3_step(delete_artifacts) == SQLITE_DONE;
		sqlite3_finalize(delete_artifacts);
	}
	else
	{
		ok = false;
	}

	sqlite3_stmt * delete_segment = nullptr;
	if (ok && sqlite3_prepare_v2(mDb, "DELETE FROM segments WHERE pack_name = ?1;", -1, &delete_segment, nullptr) == SQLITE_OK)
	{
		sqlite3_bind_text(delete_segment, 1, record.pack_name.c_str(), -1, SQLITE_TRANSIENT);
		ok = sqlite3_step(delete_segment) == SQLITE_DONE;
		sqlite3_finalize(delete_segment);
	}
	else if (ok)
	{
		ok = false;
	}

	ExecSql(mDb, ok ? "COMMIT;" : "ROLLBACK;");
	return ok;
}

long long WED_ResourceCache::QueryTotalSegmentBytesLocked() const
{
	sqlite3_stmt * stmt = nullptr;
	long long total_bytes = 0;
	if (sqlite3_prepare_v2(mDb, "SELECT COALESCE(SUM(total_bytes), 0) FROM segments;", -1, &stmt, nullptr) == SQLITE_OK)
	{
		if (sqlite3_step(stmt) == SQLITE_ROW)
			total_bytes = sqlite3_column_int64(stmt, 0);
		sqlite3_finalize(stmt);
	}
	return total_bytes;
}

long long WED_ResourceCache::ComputeEffectiveHardCapBytesLocked(long long current_total_bytes) const
{
	unsigned long long free_bytes = 0;
	unsigned long long total_bytes = 0;
	if (!QueryVolumeSpace(mRootPath, free_bytes, total_bytes))
		return kConfiguredHardCapBytes;

	const long long reserve_bytes = std::max<long long>(
		kMinFreeReserveBytes,
		std::min<long long>(kMaxFreeReserveBytes, static_cast<long long>(total_bytes / 20)));
	const long long volume_limited_cap = std::max<long long>(
		0,
		current_total_bytes + static_cast<long long>(free_bytes) - reserve_bytes);
	return std::min<long long>(kConfiguredHardCapBytes, volume_limited_cap);
}

long long WED_ResourceCache::ComputePruneTargetBytesLocked(long long effective_hard_cap_bytes) const
{
	return std::max<long long>(0, effective_hard_cap_bytes - kPruneSlackBytes);
}

bool WED_ResourceCache::EnsureCapacityForWriteLocked(long long entry_size)
{
	if (entry_size <= 0 || entry_size > kSegmentSizeBytes)
		return false;
	if (!EnsureActivePackLocked())
		return false;

	if (mActivePackSize + entry_size > kSegmentSizeBytes)
	{
		SealActivePackLocked();
		if (!CreateNewActivePackLocked())
			return false;
	}

	long long current_total = QueryTotalSegmentBytesLocked();
	long long effective_hard_cap = ComputeEffectiveHardCapBytesLocked(current_total);
	if (current_total + entry_size <= effective_hard_cap)
		return true;

	const long long prune_target = ComputePruneTargetBytesLocked(effective_hard_cap);
	const long long target_before_write = std::min<long long>(
		prune_target,
		std::max<long long>(0, effective_hard_cap - entry_size));

	while (current_total > target_before_write)
	{
		SegmentRecord record;
		if (!FindOldestSealedPackLocked(record))
			break;
		if (!DeletePackLocked(record))
			break;
		current_total = QueryTotalSegmentBytesLocked();
		effective_hard_cap = ComputeEffectiveHardCapBytesLocked(current_total);
	}

	current_total = QueryTotalSegmentBytesLocked();
	effective_hard_cap = ComputeEffectiveHardCapBytesLocked(current_total);
	return current_total + entry_size <= effective_hard_cap;
}

bool WED_ResourceCache::LookupArtifactLocked(ArtifactKind kind, const std::string& cache_key, ArtifactRecord& out_record)
{
	if (!EnsureOpenLocked())
		return false;

	sqlite3_stmt * stmt = nullptr;
	if (sqlite3_prepare_v2(mDb, "SELECT pack_name, pack_offset, pack_size FROM artifacts WHERE cache_key = ?1 AND kind = ?2;", -1, &stmt, nullptr) != SQLITE_OK)
		return false;

	sqlite3_bind_text(stmt, 1, cache_key.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, static_cast<int>(kind));

	const int step = sqlite3_step(stmt);
	if (step != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return false;
	}

	const unsigned char * pack_name_text = sqlite3_column_text(stmt, 0);
	out_record.pack_name = pack_name_text ? reinterpret_cast<const char *>(pack_name_text) : "";
	out_record.offset = sqlite3_column_int64(stmt, 1);
	out_record.size = sqlite3_column_int64(stmt, 2);
	out_record.valid = true;
	sqlite3_finalize(stmt);

	return true;
}

bool WED_ResourceCache::ReadArtifactLocked(ArtifactKind kind, const ArtifactRecord& record, std::vector<unsigned char>& out_payload)
{
	if (!record.valid)
		return false;

	const std::string pack_path = mPackDirPath + DIR_STR + record.pack_name;
	MFMemFile * file = MemFile_Open(pack_path.c_str());
	if (file == nullptr)
		return false;

	const char * begin = MemFile_GetBegin(file);
	const char * end = MemFile_GetEnd(file);
	const long long available = static_cast<long long>(end - begin);
	if (record.offset < 0 || record.size < 0 || record.offset + record.size > available)
	{
		MemFile_Close(file);
		return false;
	}

	const unsigned char * entry_begin = reinterpret_cast<const unsigned char *>(begin + record.offset);
	const unsigned char * entry_end = reinterpret_cast<const unsigned char *>(begin + record.offset + record.size);
	BlobReader reader(entry_begin, entry_end);
	PackEntryHeader header = { 0 };
	if (!reader.ReadPod(header) ||
		header.magic != kPackMagic ||
		header.kind != static_cast<unsigned int>(kind) ||
		header.payload_size > static_cast<unsigned long long>(entry_end - reader.cur))
	{
		MemFile_Close(file);
		return false;
	}

	out_payload.resize(static_cast<size_t>(header.payload_size));
	if (header.payload_size > 0)
		memcpy(out_payload.data(), reader.cur, static_cast<size_t>(header.payload_size));
	MemFile_Close(file);
	return true;
}

void WED_ResourceCache::StoreArtifactLocked(ArtifactKind kind, const SourceFingerprint& fp, int flags, int schema_version, const std::vector<unsigned char>& payload)
{
	if (!EnsureOpenLocked())
		return;

	const long long entry_size = static_cast<long long>(sizeof(PackEntryHeader) + payload.size());
	if (!EnsureCapacityForWriteLocked(entry_size))
		return;

	const std::string pack_path = mPackDirPath + DIR_STR + mActivePackName;
	FILE * file = fopen(pack_path.c_str(), "ab");
	if (file == nullptr)
		return;

	PackEntryHeader header = { 0 };
	header.magic = kPackMagic;
	header.kind = static_cast<unsigned int>(kind);
	header.payload_size = static_cast<unsigned long long>(payload.size());
	const sqlite3_int64 offset = FileOffsetEnd(file);

	const bool wrote_header = fwrite(&header, sizeof(header), 1, file) == 1;
	const bool wrote_payload = payload.empty() || fwrite(payload.data(), 1, payload.size(), file) == payload.size();
	fclose(file);
	if (!wrote_header || !wrote_payload)
		return;

	mActivePackSize += entry_size;

	const std::string cache_key = BuildCacheKey(kind, fp, flags, schema_version);
	ExecSql(mDb, "BEGIN IMMEDIATE TRANSACTION;");

	sqlite3_stmt * stmt = nullptr;
	if (sqlite3_prepare_v2(
		mDb,
		"INSERT OR REPLACE INTO artifacts (cache_key, kind, schema_version, flags, source_path, source_size, source_mtime, pack_name, pack_offset, pack_size, created_at, last_access, hit_count) "
		"VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?11, 0);",
		-1,
		&stmt,
		nullptr) != SQLITE_OK)
	{
		ExecSql(mDb, "ROLLBACK;");
		return;
	}

	const long long now = NowUnixSeconds();
	sqlite3_bind_text(stmt, 1, cache_key.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, static_cast<int>(kind));
	sqlite3_bind_int(stmt, 3, schema_version);
	sqlite3_bind_int(stmt, 4, flags);
	sqlite3_bind_text(stmt, 5, fp.normalized_path.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int64(stmt, 6, fp.size);
	sqlite3_bind_int64(stmt, 7, fp.mtime);
	sqlite3_bind_text(stmt, 8, mActivePackName.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int64(stmt, 9, offset);
	sqlite3_bind_int64(stmt, 10, static_cast<sqlite3_int64>(sizeof(header) + payload.size()));
	sqlite3_bind_int64(stmt, 11, now);
	const bool insert_ok = sqlite3_step(stmt) == SQLITE_DONE;
	sqlite3_finalize(stmt);
	if (!insert_ok)
	{
		ExecSql(mDb, "ROLLBACK;");
		return;
	}

	sqlite3_stmt * segment_stmt = nullptr;
	if (sqlite3_prepare_v2(mDb, "UPDATE segments SET total_bytes = ?2 WHERE pack_name = ?1;", -1, &segment_stmt, nullptr) != SQLITE_OK)
	{
		ExecSql(mDb, "ROLLBACK;");
		return;
	}

	sqlite3_bind_text(segment_stmt, 1, mActivePackName.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int64(segment_stmt, 2, mActivePackSize);
	const bool segment_ok = sqlite3_step(segment_stmt) == SQLITE_DONE;
	sqlite3_finalize(segment_stmt);
	ExecSql(mDb, segment_ok ? "COMMIT;" : "ROLLBACK;");
}

bool WED_ResourceCache::LoadObjMeta(const std::string& source_path, WED_ResourceCacheObjMeta& out_meta)
{
	std::lock_guard<std::mutex> guard(mMutex);
	const SourceFingerprint fp = MakeFingerprint(source_path);
	if (!fp.valid)
		return false;
	const std::string cache_key = BuildCacheKey(artifact_ObjMeta, fp, 0, kObjMetaSchemaVersion);
	ArtifactRecord record;
	if (!LookupArtifactLocked(artifact_ObjMeta, cache_key, record))
		return false;
	std::vector<unsigned char> payload;
	return ReadArtifactLocked(artifact_ObjMeta, record, payload) && DeserializeObjMeta(payload, out_meta);
}

void WED_ResourceCache::StoreObjMeta(const std::string& source_path, const WED_ResourceCacheObjMeta& meta)
{
	std::lock_guard<std::mutex> guard(mMutex);
	const SourceFingerprint fp = MakeFingerprint(source_path);
	if (!fp.valid)
		return;
	StoreArtifactLocked(artifact_ObjMeta, fp, 0, kObjMetaSchemaVersion, SerializeObjMeta(meta));
}

bool WED_ResourceCache::LoadObjGeom(const std::string& source_path, XObj8& out_obj)
{
	std::lock_guard<std::mutex> guard(mMutex);
	const SourceFingerprint fp = MakeFingerprint(source_path);
	if (!fp.valid)
		return false;
	const std::string cache_key = BuildCacheKey(artifact_ObjGeom, fp, 0, kObjGeomSchemaVersion);
	ArtifactRecord record;
	if (!LookupArtifactLocked(artifact_ObjGeom, cache_key, record))
		return false;
	std::vector<unsigned char> payload;
	return ReadArtifactLocked(artifact_ObjGeom, record, payload) && DeserializeObjGeom(payload, out_obj);
}

void WED_ResourceCache::StoreObjGeom(const std::string& source_path, const XObj8& obj)
{
	std::lock_guard<std::mutex> guard(mMutex);
	const SourceFingerprint fp = MakeFingerprint(source_path);
	if (!fp.valid)
		return;
	StoreArtifactLocked(artifact_ObjGeom, fp, 0, kObjGeomSchemaVersion, SerializeObjGeom(obj));
	StoreArtifactLocked(artifact_ObjMeta, fp, 0, kObjMetaSchemaVersion, SerializeObjMeta(ExtractObjMeta(obj)));
}

bool WED_ResourceCache::LoadPreparedTexture(const std::string& source_path, int flags, PreparedTextureImage& out_image, WED_ResourceCachePreparedTextureLoadPerf * out_perf)
{
	std::lock_guard<std::mutex> guard(mMutex);
	if (out_perf != nullptr)
		*out_perf = WED_ResourceCachePreparedTextureLoadPerf();

	const auto total_begin = out_perf != nullptr ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point();
	const SourceFingerprint fp = MakeFingerprint(source_path);
	if (!fp.valid)
		return false;
	const std::string cache_key = BuildCacheKey(artifact_TexPrepared, fp, flags, kTexPreparedSchemaVersion);
	ArtifactRecord record;
	const auto lookup_begin = out_perf != nullptr ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point();
	const bool found = LookupArtifactLocked(artifact_TexPrepared, cache_key, record);
	if (out_perf != nullptr)
		out_perf->lookup_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - lookup_begin).count();
	if (!found)
		return false;
	std::vector<unsigned char> payload;
	const auto read_begin = out_perf != nullptr ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point();
	const bool read_ok = ReadArtifactLocked(artifact_TexPrepared, record, payload);
	if (out_perf != nullptr)
		out_perf->read_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - read_begin).count();
	if (!read_ok)
		return false;
	const auto deserialize_begin = out_perf != nullptr ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point();
	const bool deserialize_ok = DeserializePreparedTexture(payload, out_image);
	if (out_perf != nullptr)
	{
		out_perf->deserialize_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - deserialize_begin).count();
		out_perf->total_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - total_begin).count();
	}
	return deserialize_ok;
}

void WED_ResourceCache::StorePreparedTexture(const std::string& source_path, int flags, const PreparedTextureImage& image)
{
	std::lock_guard<std::mutex> guard(mMutex);
	const SourceFingerprint fp = MakeFingerprint(source_path);
	if (!fp.valid)
		return;
	StoreArtifactLocked(artifact_TexPrepared, fp, flags, kTexPreparedSchemaVersion, SerializePreparedTexture(image));
}

bool WED_ResourceCache::LoadCompressedTexture(const std::string& source_path, int flags, CompressedTextureImage& out_image, WED_ResourceCachePreparedTextureLoadPerf * out_perf)
{
	std::lock_guard<std::mutex> guard(mMutex);
	if (out_perf != nullptr)
		*out_perf = WED_ResourceCachePreparedTextureLoadPerf();

	const auto total_begin = out_perf != nullptr ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point();
	const SourceFingerprint fp = MakeFingerprint(source_path);
	if (!fp.valid)
		return false;
	const std::string cache_key = BuildCacheKey(artifact_TexCompressed, fp, flags, kTexCompressedSchemaVersion);
	ArtifactRecord record;
	const auto lookup_begin = out_perf != nullptr ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point();
	const bool found = LookupArtifactLocked(artifact_TexCompressed, cache_key, record);
	if (out_perf != nullptr)
		out_perf->lookup_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - lookup_begin).count();
	if (!found)
		return false;
	std::vector<unsigned char> payload;
	const auto read_begin = out_perf != nullptr ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point();
	const bool read_ok = ReadArtifactLocked(artifact_TexCompressed, record, payload);
	if (out_perf != nullptr)
		out_perf->read_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - read_begin).count();
	if (!read_ok)
		return false;
	const auto deserialize_begin = out_perf != nullptr ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point();
	const bool deserialize_ok = DeserializeCompressedTexture(payload, out_image);
	if (out_perf != nullptr)
	{
		out_perf->deserialize_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - deserialize_begin).count();
		out_perf->total_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - total_begin).count();
	}
	return deserialize_ok;
}

void WED_ResourceCache::StoreCompressedTexture(const std::string& source_path, int flags, const CompressedTextureImage& image)
{
	std::lock_guard<std::mutex> guard(mMutex);
	const SourceFingerprint fp = MakeFingerprint(source_path);
	if (!fp.valid)
		return;
	StoreArtifactLocked(artifact_TexCompressed, fp, flags, kTexCompressedSchemaVersion, SerializeCompressedTexture(image));
}

bool WED_ResourceCache::LoadNavaidIndex(const std::string& source_signature, std::vector<unsigned char>& out_payload)
{
	std::lock_guard<std::mutex> guard(mMutex);
	const SourceFingerprint fp = MakeVirtualFingerprint(source_signature);
	if (!fp.valid)
		return false;
	const std::string cache_key = BuildCacheKey(artifact_NavaidIndex, fp, 0, kNavaidIndexSchemaVersion);
	ArtifactRecord record;
	return LookupArtifactLocked(artifact_NavaidIndex, cache_key, record) &&
		ReadArtifactLocked(artifact_NavaidIndex, record, out_payload);
}

void WED_ResourceCache::StoreNavaidIndex(const std::string& source_signature, const std::vector<unsigned char>& payload)
{
	std::lock_guard<std::mutex> guard(mMutex);
	const SourceFingerprint fp = MakeVirtualFingerprint(source_signature);
	if (!fp.valid)
		return;
	StoreArtifactLocked(artifact_NavaidIndex, fp, 0, kNavaidIndexSchemaVersion, payload);
}
