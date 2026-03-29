/*
 * Copyright (c) 2026
 */

#ifndef WED_ResourceCache_H
#define WED_ResourceCache_H

#include "TexUtils.h"
#include "XObjDefs.h"
#include <mutex>
#include <string>
#include <vector>

struct WED_ResourceCacheObjMeta {
	float fixed_heading = -1.0f;
	float viewpoint_height = -1.0f;
	float xyz_min[3] = { 0.0f, 0.0f, 0.0f };
	float xyz_max[3] = { 0.0f, 0.0f, 0.0f };
};

struct WED_ResourceCachePreparedTextureLoadPerf {
	double lookup_seconds = 0.0;
	double read_seconds = 0.0;
	double deserialize_seconds = 0.0;
	double total_seconds = 0.0;
};

class WED_ResourceCache {
public:
	static WED_ResourceCache& Get();

	bool Enabled() const;
	std::string GetRootPath() const;
	void SetEnabled(bool enabled);
	bool Clear();

	bool LoadObjMeta(const std::string& source_path, WED_ResourceCacheObjMeta& out_meta);
	void StoreObjMeta(const std::string& source_path, const WED_ResourceCacheObjMeta& meta);

	bool LoadObjGeom(const std::string& source_path, XObj8& out_obj);
	void StoreObjGeom(const std::string& source_path, const XObj8& obj);

	bool LoadPreparedTexture(const std::string& source_path, int flags, PreparedTextureImage& out_image, WED_ResourceCachePreparedTextureLoadPerf * out_perf = nullptr);
	void StorePreparedTexture(const std::string& source_path, int flags, const PreparedTextureImage& image);
	bool LoadCompressedTexture(const std::string& source_path, int flags, CompressedTextureImage& out_image, WED_ResourceCachePreparedTextureLoadPerf * out_perf = nullptr);
	void StoreCompressedTexture(const std::string& source_path, int flags, const CompressedTextureImage& image);

private:
	WED_ResourceCache();
	~WED_ResourceCache();
	WED_ResourceCache(const WED_ResourceCache&) = delete;
	WED_ResourceCache& operator=(const WED_ResourceCache&) = delete;

	enum ArtifactKind {
		artifact_ObjMeta = 1,
		artifact_ObjGeom = 2,
		artifact_TexPrepared = 3,
		artifact_TexCompressed = 4
	};

	struct SourceFingerprint {
		std::string normalized_path;
		long long size = 0;
		long long mtime = 0;
		bool valid = false;
	};

	struct ArtifactRecord {
		std::string pack_name;
		long long offset = 0;
		long long size = 0;
		bool valid = false;
	};

	struct SegmentRecord {
		std::string pack_name;
		long long sequence = 0;
		long long total_bytes = 0;
		bool sealed = false;
		bool valid = false;
	};

	bool EnsureOpenLocked();
	void CloseLocked();
	bool EnsureSchemaLocked();
	bool ResetStorageLocked();
	SourceFingerprint MakeFingerprint(const std::string& source_path) const;
	std::string BuildCacheKey(ArtifactKind kind, const SourceFingerprint& fp, int flags, int schema_version) const;
	bool LookupArtifactLocked(ArtifactKind kind, const std::string& cache_key, ArtifactRecord& out_record);
	bool ReadArtifactLocked(ArtifactKind kind, const ArtifactRecord& record, std::vector<unsigned char>& out_payload);
	void StoreArtifactLocked(ArtifactKind kind, const SourceFingerprint& fp, int flags, int schema_version, const std::vector<unsigned char>& payload);
	bool EnsureCapacityForWriteLocked(long long entry_size);
	bool EnsureActivePackLocked();
	bool CreateNewActivePackLocked();
	void SealActivePackLocked();
	bool FindOldestSealedPackLocked(SegmentRecord& out_record);
	bool DeletePackLocked(const SegmentRecord& record);
	long long QueryTotalSegmentBytesLocked() const;
	long long ComputeEffectiveHardCapBytesLocked(long long current_total_bytes) const;
	long long ComputePruneTargetBytesLocked(long long effective_hard_cap_bytes) const;

	mutable std::mutex mMutex;
	bool mEnabled;
	bool mInitAttempted;
	bool mOpenOk;
	std::string mRootPath;
	std::string mDbPath;
	std::string mPackDirPath;
	std::string mActivePackName;
	long long mActivePackSize;
	long long mNextPackSequence;
	struct sqlite3 * mDb;
};

#endif
