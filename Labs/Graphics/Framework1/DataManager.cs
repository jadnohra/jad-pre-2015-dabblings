using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Framework1
{
    // effect state management: http://www.nesnausk.org/inoutside/Technology.php
    // resource management: http://aras-p.info/texts/index.html

    class DataObject
    {
    }

    class DataStructure
    {
    }

    class RAMStreamData
    {
    }

    class RAMStream
    {
    }

    // use managed resources until there is need to do otherwize

    class ManagedVertexStreamData
    {
    }

    class ManagedVertexStream
    {
    }
    /*
    class VRAMVertexStreamData
    {
    }

    class VRAMVertexStream
    {
    }
     */

    /*
    class VRAMIndexStreamData
    {
    }

    class VRAMIndexStream
    {
    }
     */

    class ManagedIndexStreamData
    {
    }

    class ManagedIndexStream
    {
    }

    class ManagedTextureData
    {
    }

    class ManagedTexture
    {
    }

    /*
    class VRAMTextureData
    {
    }

    class VRAMTexture
    {
    }
     */ 

    class TextureAtlasMapping
    {
    }

    class Scene2
    {
        class VisibleBlock
        {
            class Renderable
            {
            }
        }
    }

    class RenderManager
    {
        class RenderJob
        {
        }

        class SceneFace
        {
            //Quake3Loader::Face FaceSourceData;
            //RenderableID RenderableID;

            class RenderJob
            {
                //VRAMTexture VRamTexture;
                RAMStream VertexStream;
                RAMStream IndexStream;
                // or
                //VRAMVertexStream VRamVertexStream;
                //VRAMIndexStream VRAMIndexStream;

                class RenderState
                {
                }
            }
            
        }
    }

    class RenderQueue2
    {
    }

    abstract class RenderJobCache
    {
        //jobs can be evicted
        //RenderJob FindJob(RenderableID ID);
    }

    abstract class TextureManager
    {
       // VRAMTexture GetVRAMTexture(string path);
    }

    abstract class DataManager
    {
        /*
        public DataStructure New();
        public void Remove(DataStructure strcuture);

        public RAMStream New(RAMStreamData data, bool evictable);
        public RAMStreamData Resolve(RAMStream stream);
        public void Restore(RAMStream stream, RAMStreamData data);
        public void Delete(RAMStream stream);

        public VRAMVertexStream New(DataStructure structure, VRAMVertexStreamData data, bool evictable);
        public VRAMVertexStreamData Resolve(VRAMVertexStream stream);
        public void Restore(VRAMVertexStream stream, VRAMVertexStreamData data);
        public void Delete(VRAMVertexStream stream);

        public VRAMIndexStream New(bool Is32Bit, VRAMIndexStreamData data, bool evictable);
        public VRAMIndexStreamData Resolve(VRAMIndexStream stream);
        public void Restore(VRAMIndexStream stream, VRAMIndexStreamData data);
        public void Delete(VRAMIndexStream stream);

        public VRAMTexture New(VRAMTextureData data, int mipLevels, bool evictable, bool enableAtlas, out TextureAtlasMapping mapping);
        public VRAMTextureData Resolve(VRAMTexture texture);
        public void Restore(VRAMTexture texture, VRAMTextureData data, int mipLevels);
        public void Delete(VRAMTexture texture);
         * */
    }
}
