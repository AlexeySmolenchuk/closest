#include <ai.h>
#include <UT/UT_DSOVersion.h>
#include <GU/GU_Detail.h>
#include <GU/GU_RayIntersect.h>
#include <GEO/GEO_Primitive.h>
#include <GEO/GEO_PrimPoly.h>
#include <GEO/GEO_PolyCounts.h>
#include <UT/UT_Vector3.h>
#include <UT/UT_Vector4.h>
#include <UT/UT_Array.h>
#include <iostream>



AI_SHADER_NODE_EXPORT_METHODS(SimpleMethods);
 
enum SimpleParams
{
    p_filename,
    p_maxdist,
    p_mode,
    p_attribute
};
 
static const char* mode[] = {
    "Distance",
    "Attribute",
    NULL
};

node_parameters
{
    AiParameterStr("filename", "example.bgeo.sc");
    AiParameterFlt("maxdist", -1);
    AiParameterEnum("mode", 0, mode);
    AiParameterStr("attribute", "Cd");
}
 

node_loader
{
    
   if (i > 0)
   {
      return false;
   }
   node->methods     = SimpleMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = "closest";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}

struct ShaderData
{
    const char* loadedGeo = "";
    const GU_RayIntersect *isect = nullptr;
    const GA_Attribute *attrib = nullptr;
    AtMatrix mesh_matrix;
};


// node_plugin_initialize
// {
//     AiMsgWarning("[closest] plugin initialized");
//     return true;
// }

node_initialize
{
    ShaderData *data = new ShaderData;
    AiNodeSetLocalData(node, data);

    GU_Detail::loadIODSOs();// Ensure all plugins are loaded
}


node_update
{

    ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
    const char* filename = AiNodeGetStr(node, "filename");

    // need to reread geo if fiename changed
    if (strcmp(data->loadedGeo, filename) != 0 )
    {
        data->loadedGeo = filename;

        if (data->isect)
        {
            // cleanup memory
            delete data->isect->detail();
            delete data->isect;    
        }

        GU_Detail *my_geo = new GU_Detail();
        my_geo->clearAndDestroy();

        // use op: syntax to get data directly from ass
        if (std::string(filename).find("op:", 0) == 0)
        {
            // AtNodeIterator *iter = AiUniverseGetNodeIterator(AI_NODE_ALL);
            // while (!AiNodeIteratorFinished(iter))
            // {
            //    AtNode *node = AiNodeIteratorGetNext(iter);
            //    // do something with node ...
            //    std::cout << "!!!! " << AiNodeGetName(node) << std::endl;
            // }
            // AiNodeIteratorDestroy(iter);
            
            std::string nodename = std::string(filename).substr(3);
            AtNode *mymesh = AiNodeLookUpByName((nodename+"/polygons").c_str());
            
            if (mymesh)
            {

                // AtParamIterator *iter = AiNodeEntryGetParamIterator(AiNodeGetNodeEntry(mymesh));
                // while (!AiParamIteratorFinished(iter))
                // {
                //    const AtParamEntry   *parm = AiParamIteratorGetNext(iter);
                //    // do something with node ...
                //    std::cout << "!!!! " << AiParamGetName(parm) << " " << AiParamGetTypeName(AiParamGetType(parm)) << std::endl;
                // }
                // AiParamIteratorDestroy(iter);

                // AtParamIterator *iter = AiNodeEntryGetParamIterator(AiNodeGetNodeEntry(mymesh));
                // while (!AiParamIteratorFinished(iter))
                // {
                //    const AtParamEntry   *parm = AiParamIteratorGetNext(iter);
                //    // do something with node ...
                //    std::cout << "!!!! " << AiParamGetName(parm) << " " << AiParamGetTypeName(AiParamGetType(parm)) << std::endl;
                // }
                // AiParamIteratorDestroy(iter);

                AtArray *vlist = AiNodeGetArray (mymesh, "vlist");
                AtArray *nsides = AiNodeGetArray (mymesh, "nsides");
                AtArray *vidxs = AiNodeGetArray (mymesh, "vidxs");

                data->mesh_matrix = AiM4Invert(AiNodeGetMatrix (mymesh, "matrix"));

                //fill gdp  from arnold node
                int num_points = AiArrayGetNumElements(vlist);

                UT_Array<UT_Vector3> positions;
                positions.setSize(num_points);

                for (int i=0; i<num_points; i++)
                {
                    AtVector p = AiArrayGetVec(vlist, i);
                    positions[i].assign(p.x, p.y, p.z);
                }

                GEO_PolyCounts polygonsizes;
                for (int i=0; i<AiArrayGetNumElements(nsides); i++)
                {
                    polygonsizes.append(AiArrayGetInt(nsides, i), 1);
                }

                UT_IntArray polygonpointnumbers;
                polygonpointnumbers.setSize(AiArrayGetNumElements(vidxs));

                for (int i=0; i<AiArrayGetNumElements(vidxs); i++)
                {
                    polygonpointnumbers[i] = AiArrayGetInt(vidxs, i);
                }

                GEO_PrimPoly::buildBlock(my_geo, positions.array() , num_points, polygonsizes, polygonpointnumbers.array(), true);

                // read Attribute from ASS
                AtString attrName = AiNodeGetStr(node, "attribute");
                if ((AiNodeGetInt(node, "mode")==1)&&(strcmp(attrName, "P")!=0))
                {
                    
                    AtArray *attr = AiNodeGetArray (mymesh, attrName);
                    // std::cout << AiArrayGetNumElements(attr)<< std::endl;
                    // std::cout <<AiParamGetTypeName(AiArrayGetType (attr))<< std::endl;
                    // std::cout <<AiParamGetTypeSize(AiArrayGetType (attr))<< std::endl;

                    const AtUserParamEntry *userparm = AiNodeLookUpUserParameter(mymesh, attrName);
                    // std::cout << AiUserParamGetName(userparm)<< std::endl;
                    // std::cout << int(AiUserParamGetType(userparm))<< std::endl;
                    // std::cout << int(AiUserParamGetArrayType(userparm))<< std::endl;
                    // std::cout << (int)AiUserParamGetCategory(userparm) << std::endl;


                    switch(AiUserParamGetCategory(userparm))
                    {
                        case AI_USERDEF_VARYING:
                        {
                            GA_RWHandleV3 cdh;
                            cdh = GA_RWHandleV3(my_geo->addFloatTuple(GA_ATTRIB_POINT, attrName.c_str(), 3, GA_Defaults(0.0)));
                            switch(AiUserParamGetType(userparm))
                            {
                                case AI_TYPE_RGB:
                                {
                                    for (int i=0; i<num_points; i++)
                                    {
                                        AtRGB val = AiArrayGetRGB(attr, i);
                                        cdh.set(GA_Offset(i), UT_Vector3(val.r, val.g, val.b));
                                    }
                                    break;
                                }
                                case AI_TYPE_VECTOR:
                                {
                                    for (int i=0; i<num_points; i++)
                                    {
                                        AtVector val = AiArrayGetVec(attr, i);
                                        cdh.set(GA_Offset(i), UT_Vector3(val.x, val.y, val.z));
                                    }
                                    break;
                                }
                                case AI_TYPE_FLOAT:
                                {
                                    for (int i=0; i<num_points; i++)
                                    {
                                        cdh.set(GA_Offset(i), UT_Vector3(AiArrayGetFlt(attr, i)));
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                        case AI_USERDEF_INDEXED:
                        {
                            char idxs_name[32];
                            sprintf(idxs_name, "%sidxs", attrName.c_str());
                            AtArray *attr_idx = AiNodeGetArray (mymesh,  idxs_name );

                            GA_RWHandleV3 cdh;
                            cdh = GA_RWHandleV3(my_geo->addFloatTuple(GA_ATTRIB_VERTEX, attrName.c_str(), 3, GA_Defaults(0.0)));

                            int num_vertices = AiArrayGetNumElements(attr_idx);
                            switch(AiUserParamGetType(userparm))
                            {
                                case AI_TYPE_RGB:
                                {
                                    for (int i=0; i<num_vertices; i++)
                                    {
                                        AtRGB val = AiArrayGetRGB(attr, AiArrayGetInt(attr_idx, i));
                                        cdh.set(GA_Offset(i), UT_Vector3(val.r, val.g, val.b));
                                    }
                                    break;
                                }
                                case AI_TYPE_VECTOR:
                                {
                                    for (int i=0; i<num_vertices; i++)
                                    {
                                        AtVector val = AiArrayGetVec(attr, AiArrayGetInt(attr_idx, i));
                                        cdh.set(GA_Offset(i), UT_Vector3(val.x, val.y, val.z));
                                    }
                                    break;
                                }
                                case AI_TYPE_FLOAT:
                                {
                                    for (int i=0; i<num_vertices; i++)
                                    {
                                        cdh.set(GA_Offset(i), UT_Vector3(AiArrayGetFlt(attr, AiArrayGetInt(attr_idx, i))));
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
            else AiMsgWarning("[closest] %s Loading Failed", filename);
                
        }
        else //try to read from file
        {
            if (my_geo->load(filename).success())
            {
                //AiMsgWarning("[closest] %s Loaded", filename);
            }
            else
            {
                AiMsgWarning("[closest] %s Loading Failed", filename);
            }
            data->mesh_matrix = AiM4Identity();
        }

        // TODO: group searh
        GA_PrimitiveGroup* grp = nullptr;

        // Flag 'picking' should be set to 1.  When set to 0, curves and surfaces will be polygonalized.
        data->isect = new GU_RayIntersect(my_geo, grp, 1, 0, 1); 

    }

    const char* attr_name = AiNodeGetStr(node, "attribute");
    if ((AiNodeGetInt(node, "mode")==1)&&(strcmp(AiNodeGetStr(node, "attribute"), "P")!=0))
    {
        auto my_geo = data->isect->detail();
        const GA_Attribute *attrib = nullptr;

        // Order of Attributes priority
        attrib = my_geo->findVertexAttribute( attr_name );
        if (!attrib)
            attrib = my_geo->findPointAttribute( attr_name );
        if (!attrib)
            attrib = my_geo->findPrimitiveAttribute( attr_name );

        if (attrib == nullptr) AiMsgWarning("Attribute %s Not Readable", attr_name);

        data->attrib = attrib;
    }
}
 

node_finish
{
    AiMsgWarning("[closest] finished");
}
 

shader_evaluate
{
    sg->out.RGB() = AtRGB(0);

    ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
    int mode = AiShaderEvalParamInt(p_mode);

    if ((mode==1)&&(data->attrib==nullptr)&&(strcmp(AiShaderEvalParamStr(p_attribute), "P")!=0)) return;

    AtVector P;
    P = AiM4PointByMatrixMult (data->mesh_matrix, sg->P);

    UT_Vector3 pos(P.x, P.y, P.z);
    UT_Vector4 result;

    float maxd = AiShaderEvalParamFlt(p_maxdist);
    if (maxd<0) maxd=1E18F;

    GU_MinInfo min_info;
    min_info.init(maxd*maxd);

    if (data->isect->minimumPoint( pos, min_info ))
    {
        if (mode==0)
        {
            result = sqrt(min_info.d); 
        }
        else if(mode==1)
        {
            // Special case for Position 
            if (strcmp(AiShaderEvalParamStr(p_attribute), "P")==0)
            {
                // TODO: remap for Packed
                // result = UT_Vector4(min_info.u1, min_info.v1, min_info.w1, 0);

                if (min_info.prim.isClosed())
                {
                    min_info.prim->evaluateInteriorPoint(result, min_info.u1, min_info.v1);
                }
                else
                {
                    // Normalize Polyline parametrization
                    min_info.prim->evaluatePoint(result, min_info.u1/(min_info.prim->getVertexCount()-1));
                }
            }
            else
            {
                auto owner = data->attrib->getOwner();

                // Primitive Attrib without interpolation
                if(owner==GA_ATTRIB_PRIMITIVE)
                {
                    // try to read vector attribute
                    GA_ROHandleV3 handle_v(data->attrib);
                    if (handle_v.isValid())
                    {
                        result = UT_Vector4(handle_v.get(min_info.prim.offset()));
                    }
                    else
                    {
                        // try to read float attribute
                        GA_ROHandleF handle_f(data->attrib);
                        if (handle_f.isValid())
                        {
                            result = handle_f.get(min_info.prim.offset());
                        }
                    }
                }

                // Point and Vertex Attributes
                if((owner==GA_ATTRIB_POINT)||(owner==GA_ATTRIB_VERTEX))
                {
                    UT_Array<GA_Offset> offsetarray;
                    UT_FloatArray weightarray;

                    if (min_info.prim.isClosed())
                    {
                        min_info.prim->computeInteriorPointWeights(offsetarray, weightarray, min_info.u1, min_info.v1, 0);
                    }
                    else
                    {
                        // Normalize Polyline parametrization
                        min_info.prim->computeInteriorPointWeights(offsetarray, weightarray, min_info.u1/(min_info.prim->getVertexCount()-1), 0, 0);
                    }
                    
                    // Do the weighted average.

                    result = 0;
                    for (exint i = 0; i < offsetarray.size(); ++i)
                    {
                        // try to read vector attribute
                        GA_ROHandleV3 handle_v(data->attrib);
                        if (handle_v.isValid())
                        {
                        // Assuming either a point or vertex normal attribute
                        GA_Offset offset = offsetarray(i);
                        if (owner == GA_ATTRIB_POINT)
                            offset = data->isect->detail()->vertexPoint(offset);
                        result += UT_Vector4(weightarray(i) * handle_v.get(offset));
                        }
                        else
                        {
                            // try to read float attribute
                            GA_ROHandleF handle_f(data->attrib);
                            if (handle_f.isValid())
                            {
                                GA_Offset offset = offsetarray(i);
                                if (owner == GA_ATTRIB_POINT)
                                    offset = data->isect->detail()->vertexPoint(offset);
                                result += weightarray(i) * handle_f.get(offset);
                                
                            }
                        }
                    }
                }
            }
        }

        sg->out.RGB() = AtRGB(result.x(), result.y(), result.z());
    }
    else
    {
        if (mode==0) sg->out.RGB() = AtRGB(maxd);
    }

}

