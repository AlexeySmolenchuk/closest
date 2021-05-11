# closest

### Description
Arnold Shader similar to **xyzdist** VEX function and **ClosestPointOnMesh** Maya command.

Shader uses **Houdini** functionality via HDK calls.

**op:** syntax can be used to sample mesh directly from rendered ass ( for example: *op:/obj/geo* )

### Usage
You can sample minimum distance to mesh, position or attribute.
Simple example. Displace by sin of distance.

![Displacement Example](/images/displace_sin_distance.jpg)

![Network Example](/images/network_example.jpg)

### Features
Feature | distance | P | PrimAttrib | PointAttrib | VertexAttrib
---|:---:|:---:|:---:|:---:|:---:
Polymesh External File | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
Polyline External File | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
Packed External File | :heavy_check_mark:
Alembic External File | :heavy_check_mark: 
Polymesh Object | :heavy_check_mark: | :heavy_check_mark:
Polyline Object |

### Build
Lniux and Windows scripts provided. You need:
- Download and specify **ARNOLD_ROOT** path to Arnold SDK
- Specify **HFS** path to Houdini instalation
- On Windows: specify **MSVCDir** path
