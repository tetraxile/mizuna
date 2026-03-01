#include "afl/bfres/reader.h"

#include "afl/bfres/results.h"
#include "afl/results.h"
#include "tinygltf/tiny_gltf.h"

namespace bfres {

hk::Result Reader::read() {
	HK_TRY(readHeader(mBase));

	return hk::ResultSuccess();
}

hk::Result Reader::readHeader(const u8* offset) {
	HK_TRY(reader::checkSignature(offset, "FRES    ", 8));

	u32 version = reader::readU32(offset + 8, util::ByteOrder::Big);

	if (version != 0x00000800) {
		u16 major = version >> 16;
		u8 minor = version >> 8;
		u8 micro = version;

		fprintf(
			stderr, "error: unsupported BFRES version (got: %d.%d.%d, expected: 8.0.0)\n", major,
			minor, micro
		);
		return ResultUnimplementedVersion();
	}

	HK_TRY(reader::readByteOrder(&mByteOrder, offset + 0xc, 0xFEFF));

	u8 alignment = reader::readU8(offset + 0xe);
	u8 targetAddrSize = reader::readU8(offset + 0xf);
	u32 filenameOffset = reader::readU32(offset + 0x10, mByteOrder);
	u16 isRelocated = reader::readU16(offset + 0x14, mByteOrder);
	u16 firstBlockOffset = reader::readU16(offset + 0x16, mByteOrder);
	u32 relocTableOffset = reader::readU32(offset + 0x18, mByteOrder);
	u32 fileSize = reader::readU32(offset + 0x1c, mByteOrder);
	u64 filenameOffsetPrefixed = reader::readU64(offset + 0x20, mByteOrder);

	u64 modelArrayOffset = reader::readU64(offset + 0x28, mByteOrder);
	u64 modelDictOffset = reader::readU64(offset + 0x30, mByteOrder);
	u64 sklAnimArrayOffset = reader::readU64(offset + 0x38, mByteOrder);
	u64 sklAnimDictOffset = reader::readU64(offset + 0x40, mByteOrder);
	u64 matAnimArrayOffset = reader::readU64(offset + 0x48, mByteOrder);
	u64 matAnimDictOffset = reader::readU64(offset + 0x50, mByteOrder);
	u64 visAnimArrayOffset = reader::readU64(offset + 0x58, mByteOrder);
	u64 visAnimDictOffset = reader::readU64(offset + 0x60, mByteOrder);
	u64 shpAnimArrayOffset = reader::readU64(offset + 0x68, mByteOrder);
	u64 shpAnimDictOffset = reader::readU64(offset + 0x70, mByteOrder);
	u64 scnAnimArrayOffset = reader::readU64(offset + 0x78, mByteOrder);
	u64 scnAnimDictOffset = reader::readU64(offset + 0x80, mByteOrder);
	u64 memoryPoolOffset = reader::readU64(offset + 0x88, mByteOrder);
	u64 bufferInfoOffset = reader::readU64(offset + 0x90, mByteOrder);
	u64 embeddedFileArrayOffset = reader::readU64(offset + 0x98, mByteOrder);
	u64 embeddedFileDictOffset = reader::readU64(offset + 0xa0, mByteOrder);
	u64 unk = reader::readU64(offset + 0xa8, mByteOrder);
	u64 stringPoolOffset = reader::readU64(offset + 0xb0, mByteOrder);
	u32 stringPoolSize = reader::readU32(offset + 0xb8, mByteOrder);
	u16 modelCount = reader::readU16(offset + 0xbc, mByteOrder);
	u16 sklAnimCount = reader::readU16(offset + 0xbe, mByteOrder);
	u16 matAnimCount = reader::readU16(offset + 0xc0, mByteOrder);
	u16 visAnimCount = reader::readU16(offset + 0xc2, mByteOrder);
	u16 shpAnimCount = reader::readU16(offset + 0xc4, mByteOrder);
	u16 scnAnimCount = reader::readU16(offset + 0xc6, mByteOrder);
	u16 embeddedFileCount = reader::readU16(offset + 0xc8, mByteOrder);

	const std::string filename = reader::readString(mBase + filenameOffset);

	printf("header:\n");
	printf("\tfilename: %s\n", filename.c_str());
	printf("\talignment: %x\n", alignment);
	printf("\ttargetAddrSize: %d\n", targetAddrSize);
	printf("\tisRelocated: %s\n", isRelocated ? "true" : "false");
	printf("\tfirstBlockOffset: %x\n", firstBlockOffset);
	printf("\trelocTableOffset: %x\n", relocTableOffset);
	printf("\tmemoryPoolOffset: %lx\n", memoryPoolOffset);
	printf("\tstringPoolOffset: %lx\n", stringPoolOffset);
	printf("\tstringPoolSize: %d\n", stringPoolSize);
	printf("\n");

	if (bufferInfoOffset) {
		printf("buffer info offset: %lx\n", bufferInfoOffset);
		mBufferInfo = new BufferInfo(this, mBase, mByteOrder);
		HK_TRY(mBufferInfo->read(mBase + bufferInfoOffset));
	}

	if (modelCount) {
		printf("models offset: %lx %lx\n", modelArrayOffset, modelDictOffset);
		printf("models: %d\n", modelCount);
		mModels = new Dict<FMDL>(this, mBase, mByteOrder);
		HK_TRY(mModels->read(modelDictOffset, modelArrayOffset));
		printf("\n");
	}

#if 0
	if (sklAnimCount) {
		printf("skl anims: %d\n", sklAnimCount);
		printf("\tarray offset: %lx (size: 0x60)\n", sklAnimArrayOffset);
		printf("\tdict offset: %lx\n", sklAnimDictOffset);
		printf("entries:\n");
		for (u16 i = 0; i < sklAnimCount; i++) {
			printf("\tskeletal animation:\n");
		}
		printf("\n");
	}

	if (matAnimCount) {
		printf("mat anims: %d\n", matAnimCount);
		printf("\tarray offset: %lx\n", matAnimArrayOffset);
		printf("\tdict offset: %lx\n", matAnimDictOffset);
		printf("\tcount: %d\n", matAnimCount);
		printf("entries:\n");
		for (u16 i = 0; i < matAnimCount; i++) {
			printf("\tmaterial animation:\n");
		}
		printf("\n");
	}

	if (visAnimCount) {
		printf("vis anims: %d\n", visAnimCount);
		printf("\tarray offset: %lx\n", visAnimArrayOffset);
		printf("\tdict offset: %lx\n", visAnimDictOffset);
		printf("entries:\n");
		for (u16 i = 0; i < visAnimCount; i++) {
			printf("\tvisibility animation:\n");
		}
		printf("\n");
	}

	if (shpAnimCount) {
		printf("shp anims: %d\n", shpAnimCount);
		printf("\tarray offset: %lx\n", shpAnimArrayOffset);
		printf("\tdict offset: %lx\n", shpAnimDictOffset);
		printf("entries:\n");
		for (u16 i = 0; i < shpAnimCount; i++) {
			printf("\tshape animation:\n");
		}
		printf("\n");
	}

	if (scnAnimCount) {
		printf("scn anims: %d\n", scnAnimCount);
		printf("\tarray offset: %lx\n", scnAnimArrayOffset);
		printf("\tdict offset: %lx\n", scnAnimDictOffset);
		printf("entries:\n");
		for (u16 i = 0; i < scnAnimCount; i++) {
			printf("\tscene animation:\n");
		}
		printf("\n");
	}

	if (embeddedFileCount) {
		printf("embedded files: %d\n", embeddedFileCount);
		printf("\tarray offset: %lx\n", embeddedFileArrayOffset);
		printf("\tdict offset: %lx\n", embeddedFileDictOffset);
		printf("entries:\n");
		for (u16 i = 0; i < embeddedFileCount; i++) {
			const u8* entryOffset = offset + embeddedFileArrayOffset + i * 0x10;
			u64 fileDataOffset = reader::readU64(entryOffset, mByteOrder);
			u32 fileDataSize = reader::readU32(entryOffset + 0x8, mByteOrder);
			printf("\tembedded file:\n");
			printf("\t\toffset: %lx\n", fileDataOffset);
			printf("\t\tsize: %x\n", fileDataSize);
		}
	}
#endif

	return hk::ResultSuccess();
}

hk::Result Reader::exportGLTF(const fs::path& output) {
	const std::string& fileExt = output.extension().string();
	bool isBinary;
	if (util::isEqual(fileExt, ".glb"))
		isBinary = true;
	else if (util::isEqual(fileExt, ".gltf"))
		isBinary = false;
	else {
		fprintf(stderr, "error: invalid file extension (expected .glb or .gltf)\n");
		return ResultInvalidFileExtension();
	}

	const std::array<u32, 4> accessorTypes = { TINYGLTF_TYPE_SCALAR, TINYGLTF_TYPE_VEC2,
		                                       TINYGLTF_TYPE_VEC3, TINYGLTF_TYPE_VEC4 };

	const std::array<u32, 3> indexFormats = { TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE,
		                                      TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT,
		                                      TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT };

	tinygltf::Model m;
	tinygltf::Scene scene;

	tinygltf::Asset asset;
	asset.version = "2.0";
	asset.generator = "afl";
	m.asset = asset;

	tinygltf::Buffer buffer;
	size_t curOffset = 0;

	FMDL* model = getModel(0);

	tinygltf::Node modelNode;
	modelNode.name = model->getName();

	for (u32 shapeIdx = 0; shapeIdx < model->getShapeCount(); shapeIdx++) {
		const FSHP* shape = model->getShape(shapeIdx);
		const FVTX* object = model->getVtxBuffer(shape->getVtxBufferIdx());

		tinygltf::Mesh mesh;
		mesh.name = shape->getName();

		tinygltf::Primitive primitive;
		primitive.mode = TINYGLTF_MODE_TRIANGLES;

		{
			const Mesh* msh = shape->getMesh(0);            // LOD 0
			const SubMesh* submesh = msh->mSubMeshes.at(0); // TODO check this

			u32 stride = getIndexFormatStride(submesh->mIdxFormat);
			u32 bufLen = submesh->mCount * stride;

			primitive.indices = m.accessors.size();

			tinygltf::Accessor accessor;
			accessor.bufferView = m.bufferViews.size();
			accessor.componentType = indexFormats.at((u32)submesh->mIdxFormat);
			accessor.count = submesh->mCount;
			accessor.type = TINYGLTF_TYPE_SCALAR;
			m.accessors.push_back(accessor);

			tinygltf::BufferView bufferView;
			bufferView.buffer = 0;
			bufferView.byteOffset = curOffset;
			bufferView.byteLength = bufLen;
			bufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
			m.bufferViews.push_back(bufferView);

			buffer.data.reserve(buffer.data.size() + bufLen);
			buffer.data.insert(buffer.data.end(), submesh->mOffset, submesh->mOffset + bufLen);
			curOffset += bufLen;
		}

		for (u32 attrIdx = 0; attrIdx < object->mAttrs->getNodeCount(); attrIdx++) {
			const VertexAttribute* attr = object->mAttrs->getValue(attrIdx);
			const VertexBuffer* buf = object->mBuffers.at(attr->mBufferIdx);
			u32 size = getAttrFormatSize(attr->mFormat);

			size_t bufLen = buf->mCount * size * 4;

			std::string attrName;
			if (attr->getName() == "_p0")
				attrName = "POSITION";
			else if (attr->getName() == "_n0")
				attrName = "NORMAL";
			else if (attr->getName() == "_t0")
				attrName = "TANGENT";
			else if (attr->getName() == "_u0")
				attrName = "TEXCOORD_0";
			else if (attr->getName() == "_u1")
				attrName = "TEXCOORD_1";
			else if (attr->getName() == "_u2")
				attrName = "TEXCOORD_2";
			else if (attr->getName() == "_c0")
				attrName = "COLOR";
			else if (attr->getName() == "_i0")
				attrName = "JOINTS_0";
			else if (attr->getName() == "_w0")
				attrName = "WEIGHTS_0";
			// else if (attr->getName() == "_b0")
			// 	attrName = "_BITANGENT";
			else {
				fprintf(
					stderr, "error: unimplemented vertex attribute '%s'\n", attr->getName().c_str()
				);
				return ResultInvalidVertexAttribute();
			}

			primitive.attributes[attrName] = m.accessors.size();

			tinygltf::Accessor accessor;
			accessor.bufferView = m.bufferViews.size();
			accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			accessor.count = buf->mCount;
			accessor.type = accessorTypes.at(size - 1);
			m.accessors.push_back(accessor);

			tinygltf::BufferView bufferView;
			bufferView.buffer = 0;
			bufferView.byteOffset = curOffset;
			bufferView.byteLength = bufLen;
			bufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
			m.bufferViews.push_back(bufferView);

			buffer.data.reserve(buffer.data.size() + bufLen);
			for (u32 v = 0; v < buf->mCount; v++) {
				const u8* vtxOffset = buf->mOffset + v * buf->mStride;
				hk::util::Vector4f data;
				HK_TRY(readAttrFormat(&data, vtxOffset, attr->mFormat, mByteOrder));

				writer::writeF32(buffer.data, curOffset, data.x, mByteOrder);
				if (size > 1) writer::writeF32(buffer.data, curOffset + 4, data.y, mByteOrder);
				if (size > 2) writer::writeF32(buffer.data, curOffset + 8, data.z, mByteOrder);
				if (size > 3) writer::writeF32(buffer.data, curOffset + 12, data.w, mByteOrder);
				curOffset += size * 4;
			}
		}

		modelNode.children.push_back(m.nodes.size());
		tinygltf::Node node;
		node.mesh = m.meshes.size();
		m.nodes.push_back(node);

		mesh.primitives.push_back(primitive);
		m.meshes.push_back(mesh);
	}

	printf("before %x, after %lx\n", getGPUBufferSize(), buffer.data.size());

	m.buffers.push_back(buffer);

	scene.nodes.push_back(m.nodes.size());
	m.nodes.push_back(modelNode);
	m.defaultScene = m.scenes.size();
	m.scenes.push_back(scene);

	tinygltf::TinyGLTF gltf;
	gltf.WriteGltfSceneToFile(&m, output.string(), true, true, true, isBinary);

	return hk::ResultSuccess();
}

} // namespace bfres
