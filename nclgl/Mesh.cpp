#include "Mesh.h"
#include "Matrix2.h"
#include "Component.h"
# define M_PI 3.14159265358979323846

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader.h"

using std::string;

Mesh::Mesh(void)	{
	glGenVertexArrays(1, &arrayObject);
	
	for(int i = 0; i < MAX_BUFFER; ++i) {
		bufferObject[i] = 0;
	}

	numVertices  = 0;
	type		 = GL_TRIANGLES;

	numIndices		= 0;
	vertices		= nullptr;
	textureCoords	= nullptr;
	normals			= nullptr;
	tangents		= nullptr;
	indices			= nullptr;
	colors			= nullptr;
	weights			= nullptr;
	weightIndices	= nullptr;
}

Mesh* Mesh::GenerateTriangle() {
    Mesh* m = new Mesh();
    m->numVertices = 3;

    m->vertices = new Vector3[m->numVertices];
    m->vertices[0] = Vector3(0.0f, 0.5f, 0.0f);
    m->vertices[1] = Vector3(0.5f, -0.5f, 0.0f);
    m->vertices[2] = Vector3(-0.5f, -0.5f, 0.0f);

    m->colors = new Vector4[m->numVertices];
    for (int i = 0; i < m->numVertices; i++) {
        m->colors[i] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    m->textureCoords = new Vector2[m->numVertices];
    m->textureCoords[0] = Vector2(0.5f, 1.0f);
    m->textureCoords[1] = Vector2(0.0f, 0.0f);
    m->textureCoords[2] = Vector2(1.0f, 0.0f);

    m->BufferData();
    return m;
}

Mesh* Mesh::GenerateQuad() {
    Mesh* m = new Mesh();
    m->numVertices = 4;
    m->type = GL_TRIANGLES;

    m->vertices = new Vector3[m->numVertices];
    m->textureCoords = new Vector2[m->numVertices];
    m->colors = new Vector4[m->numVertices];

    m->vertices[0] = Vector3(-1.0f, 1.0f, 0.0f);
    m->vertices[1] = Vector3(-1.0f, -1.0f, 0.0f);
    m->vertices[2] = Vector3(1.0f, 1.0f, 0.0f);
    m->vertices[3] = Vector3(1.0f, -1.0f, 0.0f);

    m->textureCoords[0] = Vector2(0.0f, 1.0f);
    m->textureCoords[1] = Vector2(0.0f, 0.0f);
    m->textureCoords[2] = Vector2(1.0f, 1.0f);
    m->textureCoords[3] = Vector2(1.0f, 0.0f);

    m->numIndices = 6;
    m->indices = new GLuint[m->numIndices] {
        0, 3, 2, 0, 1, 3,
    };


    for (int i = 0; i < 4; i++) {
        m->colors[i] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    m->GenerateNormals();
    m->BufferData();
    return m;
}

Mesh::~Mesh(void)	{
	glDeleteVertexArrays(1, &arrayObject);			//Delete our VAO
	glDeleteBuffers(MAX_BUFFER, bufferObject);		//Delete our VBOs

	delete[]	vertices;
	delete[]	indices;
	delete[]	textureCoords;
	delete[]	tangents;
	delete[]	normals;
	delete[]	colors;
	delete[]	weights;
	delete[]	weightIndices;
}

void Mesh::Draw()	{
	glBindVertexArray(arrayObject);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if(bufferObject[INDEX_BUFFER]) {
		glDrawElements(type, numIndices, GL_UNSIGNED_INT, 0);
	}
	else{
		glDrawArrays(type, 0, numVertices);
	}
	glBindVertexArray(0);	
}

void Mesh::DrawSubMesh(int i) {
	if (i < 0 || i >= (int)meshLayers.size()) {
		return;
	}
	SubMesh m = meshLayers[i];

	glBindVertexArray(arrayObject);
	if (bufferObject[INDEX_BUFFER]) {
		const GLvoid* offset = (const GLvoid * )(m.start * sizeof(unsigned int)); 
		glDrawElements(type, m.count, GL_UNSIGNED_INT, offset);
	}
	else {
		glDrawArrays(type, m.start, m.count);	//Draw the sampleMesh!
	}
	glBindVertexArray(0);
}

void UploadAttribute(GLuint* id, int numElements, int dataSize, int attribSize, int attribID, void* pointer, const string&debugName) {
	glGenBuffers(1, id);
	glBindBuffer(GL_ARRAY_BUFFER, *id);
	glBufferData(GL_ARRAY_BUFFER, numElements * dataSize, pointer, GL_STATIC_DRAW);

	glVertexAttribPointer(attribID, attribSize, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attribID);

	glObjectLabel(GL_BUFFER, *id, -1, debugName.c_str());
}

void	Mesh::BufferData()	{
	glBindVertexArray(arrayObject);

	////Buffer vertex data
	UploadAttribute(&bufferObject[VERTEX_BUFFER], numVertices, sizeof(Vector3), 3, VERTEX_BUFFER, vertices, "Positions");

	if(textureCoords) {	//Buffer texture data
		UploadAttribute(&bufferObject[TEXTURE_BUFFER], numVertices, sizeof(Vector2), 2, TEXTURE_BUFFER, textureCoords, "TexCoords");
	}

	if (colors) {
		UploadAttribute(&bufferObject[COLOUR_BUFFER], numVertices, sizeof(Vector4), 4, COLOUR_BUFFER, colors, "Colours");
	}

	if (normals) {	//Buffer normal data
		UploadAttribute(&bufferObject[NORMAL_BUFFER], numVertices, sizeof(Vector3), 3, NORMAL_BUFFER, normals, "Normals");
	}

	if (tangents) {	//Buffer tangent data
		UploadAttribute(&bufferObject[TANGENT_BUFFER], numVertices, sizeof(Vector4), 4, TANGENT_BUFFER, tangents, "Tangents");
	}

	if (weights) {		//Buffer weights data
		UploadAttribute(&bufferObject[WEIGHTVALUE_BUFFER], numVertices, sizeof(Vector4), 4, WEIGHTVALUE_BUFFER, weights, "Weights");
	}

	//Buffer weight indices data...uses a different function since its integers...
	if (weightIndices) {
		glGenBuffers(1, &bufferObject[WEIGHTINDEX_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[WEIGHTINDEX_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(int) * 4, weightIndices, GL_STATIC_DRAW);
		glVertexAttribIPointer(WEIGHTINDEX_BUFFER, 4, GL_INT, 0, 0); //note the new function...
		glEnableVertexAttribArray(WEIGHTINDEX_BUFFER);

		glObjectLabel(GL_BUFFER, bufferObject[WEIGHTINDEX_BUFFER], -1, "Weight Indices");
	}

	//buffer index data
	if(indices) {
		glGenBuffers(1, &bufferObject[INDEX_BUFFER]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(GLuint), indices, GL_STATIC_DRAW);

		glObjectLabel(GL_BUFFER, bufferObject[INDEX_BUFFER], -1, "Indices");
	}
	glBindVertexArray(0);	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/*
* 
* Extra file loading stuff!
* 
* */

enum class GeometryChunkTypes {
	VPositions		= 1,
	VNormals		= 2,
	VTangents		= 4,
	VColors			= 8,
	VTex0			= 16,
	VTex1			= 32,
	VWeightValues	= 64,
	VWeightIndices	= 128,
	Indices			= 256,
	JointNames		= 512,
	JointParents	= 1024,
	BindPose		= 2048,
	BindPoseInv		= 4096,
	Material		= 65536,
	SubMeshes		= 1 << 14,
	SubMeshNames	= 1 << 15
};

void ReadTextFloats(std::ifstream& file, vector<Vector2>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector2 temp;
		file >> temp.x;
		file >> temp.y;
		element.emplace_back(temp);
	}
}

void ReadTextFloats(std::ifstream& file, vector<Vector3>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector3 temp;
		file >> temp.x;
		file >> temp.y;
		file >> temp.z;
		element.emplace_back(temp);
	}
}

void ReadTextFloats(std::ifstream& file, vector<Vector4>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector4 temp;
		file >> temp.x;
		file >> temp.y;
		file >> temp.z;
		file >> temp.w;
		element.emplace_back(temp);
	}
}

void ReadTextVertexIndices(std::ifstream& file, vector<int>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		int indices[4];
		file >> indices[0];
		file >> indices[1];
		file >> indices[2];
		file >> indices[3];
		element.emplace_back(indices[0]);
		element.emplace_back(indices[1]);
		element.emplace_back(indices[2]);
		element.emplace_back(indices[3]);
	}
}

void ReadIndices(std::ifstream& file, vector<unsigned int>& elements, int numIndices) {
	for (int i = 0; i < numIndices; ++i) {
		unsigned int temp;
		file >> temp;
		elements.emplace_back(temp);
	}
}

void ReadJointParents(std::ifstream& file, vector<int>& dest) {
	int jointCount = 0;
	file >> jointCount;

	for (int i = 0; i < jointCount; ++i) {
		int id = -1;
		file >> id;
		dest.emplace_back(id);
	}
}

void ReadJointNames(std::ifstream& file, vector<string>& dest) {
	int jointCount = 0;
	file >> jointCount;
	for (int i = 0; i < jointCount; ++i) {
		std::string jointName;
		file >> jointName;
		dest.emplace_back(jointName);
	}
}

void ReadRigPose(std::ifstream& file, Matrix4** into) {
	int matCount = 0;
	file >> matCount;

	*into = new Matrix4[matCount];

	for (int i = 0; i < matCount; ++i) {
		Matrix4 mat;
		for (int i = 0; i < 16; ++i) {
			file >> mat.values[i];
		}
		(*into)[i] = mat;
	}
}

void ReadSubMeshes(std::ifstream& file, int count, vector<Mesh::SubMesh> & subMeshes) {
	for (int i = 0; i < count; ++i) {
		Mesh::SubMesh m;
		file >> m.start;
		file >> m.count;
		subMeshes.emplace_back(m);
	}
}

void ReadSubMeshNames(std::ifstream& file, int count, vector<string>& names) {
	std::string scrap;
	std::getline(file, scrap);

	for (int i = 0; i < count; ++i) {
		std::string meshName;
		std::getline(file, meshName);
		names.emplace_back(meshName);
	}
}

Mesh* Mesh::LoadFromObjFile(const char* name) {

    auto m = new Mesh();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, name, MODELPATH);

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        std::cerr << "Definitely didn't work" << std::endl;
    }

    const std::vector<tinyobj::index_t>& indices = shapes[0].mesh.indices;
    const std::vector<int>& material_ids = shapes[0].mesh.material_ids;

    m->numVertices = material_ids.size() * 3;
    m->vertices = new Vector3[m->numVertices];
    m->normals = new Vector3[m->numVertices];
    m->colors = new Vector4[m->numVertices];
    m->textureCoords = new Vector2[m->numVertices];

    m->numIndices = material_ids.size() * 3;
    m->indices = new GLuint[m->numIndices];

    GLuint iCounter = 0;
    GLuint vCounter = 0;

    // OBJ files often group verticies to save space and have multiple references to a normal/colour.
    // I'm angrily ungrouping them which probably could be made more efficient.
    for (size_t index = 0; index < material_ids.size(); ++index) {
        Vector3 original[] = {
                Vector3(attrib.vertices[indices[3 * index].vertex_index * 3],
                        attrib.vertices[indices[3 * index].vertex_index * 3 + 1],
                        attrib.vertices[indices[3 * index].vertex_index * 3 + 2]),
                Vector3(attrib.vertices[indices[3 * index + 1].vertex_index * 3],
                        attrib.vertices[indices[3 * index + 1].vertex_index * 3 + 1],
                        attrib.vertices[indices[3 * index + 1].vertex_index * 3 + 2]),
                Vector3(attrib.vertices[indices[3 * index + 2].vertex_index * 3],
                        attrib.vertices[indices[3 * index + 2].vertex_index * 3 + 1],
                        attrib.vertices[indices[3 * index + 2].vertex_index * 3 + 2])
        };

        m->vertices[vCounter] = original[0];
        m->textureCoords[vCounter] = Vector2(
                attrib.texcoords[indices[3 * index].texcoord_index * 2],
                attrib.texcoords[indices[3 * index].texcoord_index * 2 + 1]
        );
        m->colors[vCounter] = Vector4(materials[material_ids[index]].diffuse[0],materials[material_ids[index]].diffuse[1],materials[material_ids[index]].diffuse[2], 1.0);;
        m->indices[iCounter++] = vCounter++;

        m->vertices[vCounter] = original[1];
        m->textureCoords[vCounter] = Vector2(
                attrib.texcoords[indices[3 * index + 1].texcoord_index * 2],
                attrib.texcoords[indices[3 * index + 1].texcoord_index * 2 + 1]
        );
        m->colors[vCounter] = Vector4(materials[material_ids[index]].diffuse[0],materials[material_ids[index]].diffuse[1],materials[material_ids[index]].diffuse[2], 1.0);;
        m->indices[iCounter++] = vCounter++;

        m->vertices[vCounter] = original[2];
        m->textureCoords[vCounter] = Vector2(
                attrib.texcoords[indices[3 * index + 2].texcoord_index * 2],
                attrib.texcoords[indices[3 * index + 2].texcoord_index * 2 + 1]
        );
        m->colors[vCounter] = Vector4(materials[material_ids[index]].diffuse[0],materials[material_ids[index]].diffuse[1],materials[material_ids[index]].diffuse[2], 1.0);;
        m->indices[iCounter++] = vCounter++;
    }

    m->GenerateNormals();
    m->BufferData();
    return m;
}

Mesh* Mesh::LoadFromMeshFile(const string& name) {
	Mesh* mesh = new Mesh();

	std::ifstream file(MESHDIR + name);

	std::string filetype;
	int fileVersion;

	file >> filetype;

	if (filetype != "MeshGeometry") {
		std::cout << "File is not a MeshGeometry file!" << std::endl;
		return nullptr;
	}

	file >> fileVersion;

	if (fileVersion != 1) {
		std::cout << "MeshGeometry file has incompatible version!" << std::endl;
		return nullptr;
	}

	int numMeshes	= 0; //read
	int numVertices = 0; //read
	int numIndices	= 0; //read
	int numChunks	= 0; //read

	file >> numMeshes;
	file >> numVertices;
	file >> numIndices;
	file >> numChunks;

	vector<Vector3> readPositions;
	vector<Vector4> readColours;
	vector<Vector3> readNormals;
	vector<Vector4> readTangents;
	vector<Vector2> readUVs;
	vector<Vector4> readWeights;
	vector<int> readWeightIndices;

	vector<unsigned int>		readIndices;

	for (int i = 0; i < numChunks; ++i) {
		int chunkType = (int)GeometryChunkTypes::VPositions;

		file >> chunkType;

		switch ((GeometryChunkTypes)chunkType) {
		case GeometryChunkTypes::VPositions:ReadTextFloats(file, readPositions, numVertices);  break;
		case GeometryChunkTypes::VColors:	ReadTextFloats(file, readColours, numVertices);  break;
		case GeometryChunkTypes::VNormals:	ReadTextFloats(file, readNormals, numVertices);  break;
		case GeometryChunkTypes::VTangents:	ReadTextFloats(file, readTangents, numVertices);  break;
		case GeometryChunkTypes::VTex0:		ReadTextFloats(file, readUVs, numVertices);  break;
		case GeometryChunkTypes::Indices:	ReadIndices(file, readIndices, numIndices); break;

		case GeometryChunkTypes::VWeightValues:		ReadTextFloats(file, readWeights, numVertices);  break;
		case GeometryChunkTypes::VWeightIndices:	ReadTextVertexIndices(file, readWeightIndices, numVertices);  break;
		case GeometryChunkTypes::JointNames:		ReadJointNames(file, mesh->jointNames);  break;
		case GeometryChunkTypes::JointParents:		ReadJointParents(file, mesh->jointParents);  break;
		case GeometryChunkTypes::BindPose:			ReadRigPose(file, &mesh->bindPose);  break;
		case GeometryChunkTypes::BindPoseInv:		ReadRigPose(file, &mesh->inverseBindPose);  break;
		case GeometryChunkTypes::SubMeshes: 		ReadSubMeshes(file, numMeshes, mesh->meshLayers); break;
		case GeometryChunkTypes::SubMeshNames: 		ReadSubMeshNames(file, numMeshes, mesh->layerNames); break;
		}
	}
	//Now that the data has been read, we can shove it into the actual Mesh object

	mesh->numVertices	= numVertices;
	mesh->numIndices	= numIndices;

	if (!readPositions.empty()) {
		mesh->vertices = new Vector3[numVertices];
		memcpy(mesh->vertices, readPositions.data(), numVertices * sizeof(Vector3));
	}

	if (!readColours.empty()) {
		mesh->colors = new Vector4[numVertices];
		memcpy(mesh->colors, readColours.data(), numVertices * sizeof(Vector4));
	}

	if (!readNormals.empty()) {
		mesh->normals = new Vector3[numVertices];
		memcpy(mesh->normals, readNormals.data(), numVertices * sizeof(Vector3));
	}

	if (!readTangents.empty()) {
		mesh->tangents = new Vector4[numVertices];
		memcpy(mesh->tangents, readTangents.data(), numVertices * sizeof(Vector4));
	}

	if (!readUVs.empty()) {
		mesh->textureCoords = new Vector2[numVertices];
		memcpy(mesh->textureCoords, readUVs.data(), numVertices * sizeof(Vector2));
	}
	if (!readIndices.empty()) {
		mesh->indices = new unsigned int[numIndices];
		memcpy(mesh->indices, readIndices.data(), numIndices * sizeof(unsigned int));
	}

	if (!readWeights.empty()) {
		mesh->weights = new Vector4[numVertices];
		memcpy(mesh->weights, readWeights.data(), numVertices * sizeof(Vector4));
	}

	if (!readWeightIndices.empty()) {
		mesh->weightIndices = new int[numVertices * 4];
		memcpy(mesh->weightIndices, readWeightIndices.data(), numVertices * sizeof(int) * 4);
	}

	mesh->BufferData();

	return mesh;
}

int Mesh::GetIndexForJoint(const std::string& name) const {
	for (unsigned int i = 0; i < jointNames.size(); ++i) {
		if (jointNames[i] == name) {
			return i;
		}
	}
	return -1;
}

int Mesh::GetParentForJoint(const std::string& name) const {
	int i = GetIndexForJoint(name);
	if (i == -1) {
		return -1;
	}
	return jointParents[i];
}

int Mesh::GetParentForJoint(int i) const {
	if (i == -1 || i >= (int)jointParents.size()) {
		return -1;
	}
	return jointParents[i];
}

bool Mesh::GetSubMesh(int i, const SubMesh* s) const {
	if (i < 0 || i >= (int)meshLayers.size()) {
		return false;
	}
	s = &meshLayers[i];
	return true;
}

bool Mesh::GetSubMesh(const string& name, const SubMesh* s) const {
	for (unsigned int i = 0; i < layerNames.size(); ++i) {
		if (layerNames[i] == name) {
			return GetSubMesh(i, s);
		}
	}
	return false;
}

void Mesh::GenerateNormals() {
    if (!normals) {
        normals = new Vector3[numVertices];
    }

    for (GLuint i = 0; i < numVertices; i++) {
        normals[i] = Vector3();
    }

    int triCount = GetTriCount();

    for (int i = 0; i < triCount; i++) {
        unsigned int a = 0;
        unsigned int b = 0;
        unsigned int c = 0;
        GetVertexIndicesForTri(i, a, b, c);

        Vector3 normal = Vector3::Cross((vertices[b] - vertices[a]), (vertices[c] - vertices[a]));
        normals[a] += normal;
        normals[b] += normal;
        normals[c] += normal;
    }

    for(GLuint i = 0; i < numVertices; i++) {
        normals[i].Normalise();
    }
}

bool Mesh::GetVertexIndicesForTri(unsigned int i, unsigned int &a, unsigned int &b, unsigned int &c) const {
    unsigned int triCount = GetTriCount();
    if (i >= triCount) {
        return false;
    }

    if (numIndices > 0) {
        a = indices[(i * 3)];
        b = indices[(i * 3) + 1];
        c = indices[(i * 3) + 2];
    } else {
        a = (i * 3);
        b = (i * 3) + 1;
        c = (i * 3) + 2;
    }

    return true;
}

ModelData Mesh::ToModelData(Mesh *m) {
    ModelData d = {
            0,
            nullptr,
            m->arrayObject,
            m->type,
            m->numIndices
    };
    return d;
}

Mesh *Mesh::GenerateUVSphere(int slices, int stacks) {
    if(slices < 3)
        slices = 3;
    if(stacks < 2)
        stacks = 2;

    int vertPointer = 0;
    int indPointer = 0;
    int uvPointer = 0;
    int normalPointer = 0;

    Mesh *m = new Mesh;
    m->numVertices = 4000;
    m->vertices = new Vector3[m->numVertices];
    m->textureCoords = new Vector2[m->numVertices];
    m->colors = new Vector4[m->numVertices];
    m->normals = new Vector3[m->numVertices];
    int numTriangles = slices * stacks * 2;
    m->numIndices = numTriangles * 3;
    m->indices = new GLuint[12000];


    float nx, ny, nz, lengthInv = 1.0f;    // normal
    struct Vertex
    {
        float x, y, z, s, t; // Postion and Texcoords
    };

    float deltaLatitude = M_PI / stacks;
    float deltaLongitude = 2 * M_PI / slices;
    float latitudeAngle;
    float longitudeAngle;

    for (int i = 0; i <= stacks; ++i)
    {
        latitudeAngle = M_PI / 2 - i * deltaLatitude;
        float xy = cosf(latitudeAngle);
        float z = sinf(latitudeAngle);


        for (int j = 0; j <= slices; ++j)
        {
            longitudeAngle = j * deltaLongitude;

            Vertex vertex;
            vertex.x = xy * cosf(longitudeAngle);
            vertex.y = xy * sinf(longitudeAngle);
            vertex.z = z;
            vertex.s = (vertex.x + 1.0f) * 0.5f;
            vertex.t = (vertex.y + 1.0f) * 0.5f;
//            vertex.s = (float)j/slices;
//            vertex.t = (float)i/stacks;
            m->vertices[vertPointer++] = Vector3(vertex.x, vertex.y, vertex.z);
            m->textureCoords[uvPointer++] = Vector2(vertex.s, vertex.t);

            nx = vertex.x * lengthInv;
            ny = vertex.y * lengthInv;
            nz = vertex.z * lengthInv;
            m->normals[normalPointer++] = Vector3(nx, ny, nz);
        }
    }

    unsigned int k1, k2;
    for(int i = 0; i < stacks; ++i)
    {
        k1 = i * (slices + 1);
        k2 = k1 + slices + 1;
        for(int j = 0; j < slices; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                m->indices[indPointer++] = k1;
                m->indices[indPointer++] = k2;
                m->indices[indPointer++] = k1 + 1;
            }

            if (i != (stacks - 1))
            {
                m->indices[indPointer++] = k1 + 1;
                m->indices[indPointer++] = k2;
                m->indices[indPointer++] = k2 + 1;
            }
        }
    }
    m->numIndices = indPointer;
    m->numVertices = vertPointer;
    m->BufferData();
    return m;
}

void Mesh::SetColor(float r, float g, float b, float a) {
    glBindVertexArray(arrayObject);
    if (!numVertices) {
        return;
    }

    for (int i = 0; i < numVertices; i++) {
        colors[i] = Vector4(r, g, b, a);
    }

    UploadAttribute(&bufferObject[COLOUR_BUFFER], numVertices, sizeof(Vector4), 4, COLOUR_BUFFER, colors, "Colours");
}
