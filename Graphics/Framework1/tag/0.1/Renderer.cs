using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;

namespace Framework1
{
    public abstract class RenderJob
    {
        abstract public UInt32 GetSortKey();
        abstract public void Execute(Renderer renderer);
    };

    public abstract class Renderable
    {
        abstract public RenderJob GetFirstJob();
        abstract public RenderJob GetNextJob(RenderJob prevJob);
    };

    public abstract class RenderQueue
    {
        abstract public void Push(Renderable renderable);

        abstract public RenderJob GetFirstJob();
        abstract public RenderJob GetNextJob(RenderJob prevJob);
    };

    public abstract class Renderer
    {
        abstract public void Push(RenderQueue queue);

        public GraphicsDevice Device;
        //public AssetInterpreter AssetInterpreter;
    };
     
}
