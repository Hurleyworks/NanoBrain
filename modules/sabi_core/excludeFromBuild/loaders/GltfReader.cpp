#include "GltfReader.h"

using Eigen::Vector3f;
using sabi::Surface;

void GltfReader::read (const std::filesystem::path& filePath)
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
            MeshBuffers mesh;

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

                    // Search for the node referencing the current mesh
                    Eigen::Affine3f transform = Eigen::Affine3f::Identity();
                    for (cgltf_size k = 0; k < data->nodes_count; ++k)
                    {
                        if (data->nodes[k].mesh == &cgltfMesh)
                        {
                            cgltf_node* node = &data->nodes[k];
                            while (node)
                            {
                                Eigen::Affine3f localTransform = Eigen::Affine3f::Identity();
                                localTransform.translate (Eigen::Vector3f (node->translation[0], node->translation[1], node->translation[2]));
                                localTransform.rotate (Eigen::Quaternionf (node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]));
                                localTransform.scale (Eigen::Vector3f (node->scale[0], node->scale[1], node->scale[2]));

                                transform = localTransform * transform;
                                node = node->parent;
                            }
                            break; // Exit the loop once the node is found
                        }
                    }

                    mesh.transform = transform;
                }

                // Fill in vertex and normal data
                if (vertexAccessor)
                {
                    mesh.V.resize (3, vertexAccessor->count);
                    getVertexAttributes (mesh.V, vertexAccessor);
                }

                if (normalAccessor)
                {
                    mesh.N.resize (3, normalAccessor->count);
                    getVertexAttributes (mesh.N, normalAccessor);
                }

                // Fill in index data
                if (indexAccessor)
                {
                    surface.F.resize (3, indexAccessor->count / 3); // Assuming triangles
                    getTriangleIndices (surface.F, indexAccessor);
                }

                // Fill in uv data
                if (uvAccessor)
                {
                    surface.uvs.resize (uvAccessor->count);
                    getUVs (surface.uvs, uvAccessor);
                }

                // Fill in material data
                surface.material = *primitive.material;

                // Add the surface to the list of surfaces for this mesh
                mesh.surfaces.push_back (surface);
            }

            // Search for the node referencing the current mesh
            Eigen::Affine3f transform = Eigen::Affine3f::Identity();
            for (cgltf_size k = 0; k < data->nodes_count; ++k)
            {
                if (data->nodes[k].mesh == &cgltfMesh)
                {
                    cgltf_node* node = &data->nodes[k];
                    while (node)
                    {
                        Eigen::Affine3f localTransform = Eigen::Affine3f::Identity();

                        if (node->has_matrix)
                        {
                            localTransform.matrix() << node->matrix[0], node->matrix[1], node->matrix[2], node->matrix[3],
                                node->matrix[4], node->matrix[5], node->matrix[6], node->matrix[7],
                                node->matrix[8], node->matrix[9], node->matrix[10], node->matrix[11],
                                node->matrix[12], node->matrix[13], node->matrix[14], node->matrix[15];
                        }
                        else
                        {
                            localTransform.translate (Eigen::Vector3f (node->translation[0], node->translation[1], node->translation[2]));
                            localTransform.rotate (Eigen::Quaternionf (node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]));
                            localTransform.scale (Eigen::Vector3f (node->scale[0], node->scale[1], node->scale[2]));
                        }

                        transform = localTransform * transform;
                        node = node->parent;
                    }
                    break; // Exit the loop once the node is found
                }
            }
            //mace::matStr4f (transform, DBUG, "Mesh transform");

            // Store the world transform for this mesh
            mesh.transform = transform;

            // Add to the list of all mesh mesh
            meshBuffers.push_back (mesh);
        }
    }
    else
    {
        LOG (DBUG) << "Failed to read GLTF file: " << filePath;
    }
}

void GltfReader::debugMaterial (const cgltf_material* material)
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

void GltfReader::debug()
{
    for (size_t m = 0; m < meshBuffers.size(); ++m)
    {
        const MeshBuffers& mesh = meshBuffers[m];

        LOG (DBUG) << " --- MeshBuffers " << m;

        LOG (DBUG) << "  Vertices:";
        for (int i = 0; i < mesh.V.cols(); ++i)
        {
            LOG (DBUG) << "Vertex " << i << ": (" << mesh.V (0, i) << ", " << mesh.V (1, i) << ", " << mesh.V (2, i) << ")";
        }

        LOG (DBUG) << "  Normals:";
        for (int i = 0; i < mesh.N.cols(); ++i)
        {
            LOG (DBUG) << "Normal " << i << ": (" << mesh.N (0, i) << ", " << mesh.N (1, i) << ", " << mesh.N (2, i) << ")";
        }

        LOG (DBUG) << "  Surfaces:";
        for (size_t s = 0; s < mesh.surfaces.size(); ++s)
        {
            const Surface& surface = mesh.surfaces[s];

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

void GltfReader::getUVs (std::vector<Vector2f>& vec, cgltf_accessor* accessor)
{
    size_t numUVs = accessor->count;
    std::vector<float> temp (numUVs * 2);
    cgltf_size num_floats = cgltf_accessor_unpack_floats (accessor, &temp[0], temp.size());

    if (num_floats != temp.size())
    {
        LOG (DBUG) << "Failed to unpack all floats. Expected " << temp.size() << ", got " << num_floats;
        return;
    }

    vec.resize (numUVs);
    for (size_t i = 0; i < numUVs; ++i)
    {
        float u = (float)temp[i * 2];
        float v = (float)temp[i * 2 + 1];
        vec[i] = Vector2f (u, v);
    }
}
void GltfReader::getTriangleIndices (MatrixXu& matrix, cgltf_accessor* accessor)
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

void GltfReader::getVertexAttributes (Eigen::MatrixXf& matrix, const cgltf_accessor* accessor)
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
