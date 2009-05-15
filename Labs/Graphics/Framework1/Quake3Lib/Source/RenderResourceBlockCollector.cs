using System.Diagnostics;
using System.Collections.Generic;
using BlackRice.Framework.Graphics;

namespace BlackRice.Framework.Quake3
{
    /*
     * Holds the block of render resources needed to render a scene renderable
     * The renderable might need many resources, it is possible to flexibly combine 
     * resources from RAM/VRAM if needed, but also a simpler more hard coded design is possible
     * this is left to the implementation
     * Here we simply cache the resource blocks and have the necessary information to 
     * evict old ones if not done manually
     **/
    public class RenderResourceBlockCollector
    {
        public bool EvictIfNeeded(ResourceBlock block)
        {
            return false;
        }

        public abstract class ResourceBlock
        {
            public abstract void Evict(RenderResourceManager resMan);
            public abstract void Prepare(RenderResourceManager resMan);
        }
    }
}