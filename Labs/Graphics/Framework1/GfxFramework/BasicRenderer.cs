using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;

namespace Framework1
{
    public interface IBasicRenderable
    {
        void PushJobs(BasicRenderQueue queue, BasicRenderer renderer);
    };

    public interface IBasicRenderJob
    {
        //public UInt32 GetSortKey() { return 0; }
        void Execute(BasicRenderer renderer);
    };

    public class BasicRenderQueue
    {
        BasicRenderer m_Renderer;
        Queue<IBasicRenderJob> m_Jobs = new Queue<IBasicRenderJob>();

        public BasicRenderQueue(BasicRenderer renderer)
        {
            m_Renderer = renderer;
        }

        public void Push(IBasicRenderable renderable)
        {
            renderable.PushJobs(this, m_Renderer);
        }

        public void Push(IBasicRenderJob job)
        {
            m_Jobs.Enqueue(job);
        }

        public int Count() { return m_Jobs.Count; }
        public IBasicRenderJob GetNextJob() { return m_Jobs.Dequeue(); }
    };

    public class BasicRenderer
    {
        public GraphicsDevice Device;
        Queue<BasicRenderQueue> m_Queues = new Queue<BasicRenderQueue>();
        EffectContext m_EffectContext = new EffectContext();
        Effect m_Effect;

        public BasicRenderer(ContentManager content, GraphicsDevice device)
        {
            Device = device;
            //m_Effect = content.Load<Effect>("Effects/AmbientTexturedLightmapped");
            m_Effect = content.Load<Effect>("Effects/AmbientTextured");
        }

        public EffectContext getEffectContext()
        {
            return m_EffectContext;
        }

        public void Push(BasicRenderQueue queue)
        {
            m_Queues.Enqueue(queue);
        }

        public void Render()
        {
            Device.RenderState.DepthBufferEnable = true;
            {
                m_Effect.Begin();
                m_Effect.CurrentTechnique.Passes[0].Begin();

                m_Effect.Parameters[0].SetValue(m_EffectContext.GetViewProjectionMatrix());
                
                {
                    int queueCount = m_Queues.Count;

                    for (int qi = 0; qi < queueCount; ++qi)
                    {
                        BasicRenderQueue queue = m_Queues.Dequeue();

                        int jobCount = queue.Count();
                        for (int ji = 0; ji < jobCount; ++ji)
                        {
                            queue.GetNextJob().Execute(this);
                        }
                    }
                }

                m_Effect.CurrentTechnique.Passes[0].End();
                m_Effect.End();
            }
        }
    }
}
