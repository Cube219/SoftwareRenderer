#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <tuple>
#include "geometry.h"

struct FaceInfo
{
	int vertIndex;
	int uvIndex;
	int normIndex;
};

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> uvs_;
	std::vector<std::tuple<FaceInfo, FaceInfo, FaceInfo>> faces_;
	std::vector<Vec3f> norms_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec2f uv(int i);
	Vec3f normal(int i);
	std::tuple<FaceInfo, FaceInfo, FaceInfo> face(int idx);
};

#endif //__MODEL_H__
