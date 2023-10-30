//
// Created by c3042750 on 26/10/2023.
//

#ifndef SIXTH_HEIGHTMAP_H
#define SIXTH_HEIGHTMAP_H

#include "Mesh.h"
#include "stb/stb_image.h"

class HeightMap : public Mesh {
public:
    HeightMap(const std::string& name);
    ~HeightMap() = default;

    Vector3 GetHeightMapSize() const { return heightmapSize; }
protected:
    Vector3 heightmapSize;
};


#endif //SIXTH_HEIGHTMAP_H
