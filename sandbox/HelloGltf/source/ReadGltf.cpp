#include "ReadGltf.h"

void ReadGltf::read (const std::filesystem::path& filePath)
{
    cgltf_options options = {};
    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse_file (&options, filePath.string().c_str(), &data);

    if (result == cgltf_result_success)
    {
        result = cgltf_load_buffers (&options, data, filePath.string().c_str());
        if (result != cgltf_result_success)
        {
            LOG (DBUG) << "Failed to load external buffers.";
            cgltf_free (data);
            return;
        }

        // Loop through each mesh
        for (cgltf_size i = 0; i < data->meshes_count; ++i)
        {
            cgltf_mesh& cgltfMesh = data->meshes[i];
            MeshBuffers buffers;

            // Loop through each primitive in the mesh
            for (cgltf_size j = 0; j < cgltfMesh.primitives_count; ++j)
            {
                cgltf_primitive& primitive = cgltfMesh.primitives[j];
                Surface surface;

                cgltf_accessor* vertexAccessor = nullptr;
                cgltf_accessor* normalAccessor = nullptr;
                cgltf_accessor* uvAccessor = nullptr;
                cgltf_accessor* indexAccessor = primitive.indices;

                // Loop through each attribute in the primitive
                for (cgltf_size k = 0; k < primitive.attributes_count; ++k)
                {
                    cgltf_attribute& attribute = primitive.attributes[k];

                    if (attribute.type == cgltf_attribute_type_position)
                    {
                        vertexAccessor = attribute.data;
                    }
                    else if (attribute.type == cgltf_attribute_type_normal)
                    {
                        normalAccessor = attribute.data;
                    }
                    else if (attribute.type == cgltf_attribute_type_texcoord)
                    {
                        uvAccessor = attribute.data;
                    }
                }

                // Fill in vertex and normal data
                if (vertexAccessor)
                {
                    buffers.V.resize (3, vertexAccessor->count);
                    getVertexAttributes (buffers.V, vertexAccessor);
                }

                if (normalAccessor)
                {
                    buffers.N.resize (3, normalAccessor->count);
                    getVertexAttributes (buffers.N, normalAccessor);
                }

                // Fill in index data
                if (indexAccessor)
                {
                    surface.F.resize (3, indexAccessor->count / 3); // Assuming triangles
                    getTriangleIndices (surface.F, indexAccessor);

                    for (int i = 0; i < surface.F.cols(); ++i)
                    {
                        Vector3u tri = surface.F.col (i);
                        // LOG (DBUG) << tri.x() << ", " << tri.y() << ", " << tri.z();
                    }
                }
                // Fill in uv data
                if (uvAccessor)
                {
                    surface.uvs.resize (uvAccessor->count);
                    getUVs (surface.uvs, uvAccessor);

                    for (auto& uv : surface.uvs)
                    {
                        // LOG (DBUG) << uv.x() << ", " << uv.y();
                    }
                }

                // Fill in material data
                surface.material = *primitive.material;

                // Add the surface to the list of surfaces for this mesh
                buffers.surfaces.push_back (surface);
            }

            // Add to the list of all mesh buffers
            meshBuffers.push_back (buffers);
        }
    }
    else
    {
        LOG (DBUG) << "Failed to read GLTF file: " << filePath;
    }
}

void ReadGltf::debugMaterial (const cgltf_material* material)
{
    if (material == nullptr)
    {
        LOG (DBUG) << "Material is null.";
        return;
    }

    LOG (DBUG) << "Material Name: " << (material->name ? material->name : "Unnamed");

    if (material->has_pbr_metallic_roughness)
    {
        LOG (DBUG) << "Base Color Factor: ("
                   << material->pbr_metallic_roughness.base_color_factor[0] << ", "
                   << material->pbr_metallic_roughness.base_color_factor[1] << ", "
                   << material->pbr_metallic_roughness.base_color_factor[2] << ", "
                   << material->pbr_metallic_roughness.base_color_factor[3] << ")";

        LOG (DBUG) << "Metallic Factor: " << material->pbr_metallic_roughness.metallic_factor;
        LOG (DBUG) << "Roughness Factor: " << material->pbr_metallic_roughness.roughness_factor;
    }

    if (material->normal_texture.texture)
    {
        LOG (DBUG) << "Normal Texture: " << (material->normal_texture.texture->name ? material->normal_texture.texture->name : "Unnamed");
    }

    // Add more fields as needed
}

void ReadGltf::debug()
{
    for (size_t m = 0; m < meshBuffers.size(); ++m)
    {
        const MeshBuffers& buffers = meshBuffers[m];

        LOG (DBUG) << " --- MeshBuffers " << m;

        LOG (DBUG) << "  Vertices:";
        for (int i = 0; i < buffers.V.cols(); ++i)
        {
            LOG (DBUG) << "Vertex " << i << ": (" << buffers.V (0, i) << ", " << buffers.V (1, i) << ", " << buffers.V (2, i) << ")";
        }

        LOG (DBUG) << "  Normals:";
        for (int i = 0; i < buffers.N.cols(); ++i)
        {
            LOG (DBUG) << "Normal " << i << ": (" << buffers.N (0, i) << ", " << buffers.N (1, i) << ", " << buffers.N (2, i) << ")";
        }

        LOG (DBUG) << "  Surfaces:";
        for (size_t s = 0; s < buffers.surfaces.size(); ++s)
        {
            const Surface& surface = buffers.surfaces[s];

            LOG (DBUG) << "  Surface " << s;
            debugMaterial (&surface.material);

            LOG (DBUG) << "  Indices:";
            for (int i = 0; i < surface.F.cols(); ++i)
            {
                LOG (DBUG) << "Triangle " << i << ": (" << surface.F (0, i) << ", " << surface.F (1, i) << ", " << surface.F (2, i) << ")";
            }

            LOG (DBUG) << "  UVs:";
            for (size_t i = 0; i < surface.uvs.size(); ++i)
            {
                LOG (DBUG) << "UV " << i << ": (" << surface.uvs[i].x() << ", " << surface.uvs[i].y() << ")";
            }
        }
    }
}

void ReadGltf::getUVs (std::vector<Vector2f>& vec, cgltf_accessor* accessor)
{
    if (accessor == nullptr)
    {
        LOG (DBUG) << "Invalid accessor.";
        return;
    }

    if (accessor->buffer_view == nullptr || accessor->buffer_view->buffer == nullptr)
    {
        LOG (DBUG) << "Invalid buffer or buffer view.";
        return;
    }

    if (accessor->type != cgltf_type_vec2 || accessor->component_type != cgltf_component_type_r_32f)
    {
        LOG (DBUG) << "Accessor is not of expected type for UV coordinates.";
        return;
    }

    // Allocate temporary storage for the UV data
    std::vector<float> temp (accessor->count * 2); // 2 floats per UV coordinate
    cgltf_size num_floats = cgltf_accessor_unpack_floats (accessor, &temp[0], temp.size());

    if (num_floats != temp.size())
    {
        LOG (DBUG) << "Failed to unpack all floats. Expected " << temp.size() << ", got " << num_floats;
        return;
    }

    // Fill the std::vector<Eigen::Vector2f> with unpacked UV data
    vec.resize (accessor->count);
    for (size_t i = 0; i < accessor->count; ++i)
    {
        vec[i] = Vector2f (temp[i * 2], temp[i * 2 + 1]);
    }
}

void ReadGltf::getTriangleIndices (MatrixXu& matrix, cgltf_accessor* accessor)
{
    if (accessor == nullptr)
    {
        LOG (DBUG) << "Invalid accessor.";
        return;
    }

    if (accessor->buffer_view == nullptr || accessor->buffer_view->buffer == nullptr)
    {
        LOG (DBUG) << "Invalid buffer or buffer view.";
        return;
    }

    size_t numTriangles = accessor->count / 3;
    matrix.resize (3, numTriangles);

    // Corrected stride calculation
    cgltf_size stride = accessor->stride ? accessor->stride : cgltf_size (accessor->component_type);

    uint8_t* buffer = (uint8_t*)accessor->buffer_view->buffer->data;
    uint8_t* offset_ptr = buffer + accessor->offset + accessor->buffer_view->offset;

    if (accessor->component_type == cgltf_component_type_r_16u)
    {
        for (size_t i = 0; i < numTriangles; ++i)
        {
            uint16_t* idx = reinterpret_cast<uint16_t*> (offset_ptr + i * 3 * stride);
            for (size_t j = 0; j < 3; ++j)
            {
                matrix (j, i) = static_cast<unsigned int> (idx[j]);
            }
        }
    }
    else if (accessor->component_type == cgltf_component_type_r_32u)
    {
        for (size_t i = 0; i < numTriangles; ++i)
        {
            uint32_t* idx = reinterpret_cast<uint32_t*> (offset_ptr + i * 3 * stride);
            for (size_t j = 0; j < 3; ++j)
            {
                matrix (j, i) = idx[j];
            }
        }
    }
    else
    {
        LOG (DBUG) << "Unsupported index component type.";
    }
}

void ReadGltf::getVertexAttributes (Eigen::MatrixXf& matrix, const cgltf_accessor* accessor)
{
    if (accessor == nullptr)
    {
        LOG (DBUG) << "Invalid accessor.";
        return;
    }

    if (accessor->buffer_view == nullptr || accessor->buffer_view->buffer == nullptr)
    {
        LOG (DBUG) << "Invalid buffer or buffer view.";
        return;
    }

    size_t components = cgltf_num_components (accessor->type);
    if (components != matrix.rows())
    {
        LOG (DBUG) << "Accessor components do not match matrix rows.";
        return;
    }

    std::vector<float> temp (components * accessor->count);
    cgltf_size num_floats = cgltf_accessor_unpack_floats (accessor, &temp[0], temp.size());

    if (num_floats != temp.size())
    {
        LOG (DBUG) << "Failed to unpack all floats. Expected " << temp.size() << ", got " << num_floats;
        return;
    }

    for (size_t i = 0; i < components; ++i)
    {
        for (size_t j = 0; j < accessor->count; ++j)
        {
            matrix (i, j) = temp[j * components + i];
        }
    }
}
