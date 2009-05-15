using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;

namespace BlackRice.Framework.Quake3App
{
    class Stats
    {
        public Stats()
        {
            frameCounterStartTimeMillis = -1.0f;
            framePerSec = 0.0f;
            frameCount = 0;
        }

        public void LoadContent(GraphicsDevice device, ContentManager content)
        {
            spriteFont = content.Load<SpriteFont>("Fonts/DebugSpriteFont");
            spriteBatch = new SpriteBatch(device);
        }

        public void UnloadContent(GraphicsDevice device, ContentManager content)
        {
        }

        public void Update(GameTime gameTime)
        {
            frameCounterTimeMillis += (float) gameTime.ElapsedGameTime.TotalMilliseconds;

            if (frameCounterStartTimeMillis < 0.0f)
                frameCounterStartTimeMillis = frameCounterTimeMillis;
            
            ++frameCount;

            if (frameCounterTimeMillis - frameCounterStartTimeMillis > 1000.0f)
            {
                framePerSec = ((float) frameCount * 1000.0f) / ((frameCounterTimeMillis - frameCounterStartTimeMillis));
                frameCount = 0;
                frameCounterStartTimeMillis = frameCounterTimeMillis;
            }
        }

        public void Draw()
        {
            string fps = string.Format("fps: {0:F2}", framePerSec);

            spriteBatch.Begin();

            spriteBatch.DrawString(spriteFont, fps, new Vector2(0, 0), Color.GreenYellow);
            //spriteBatch.DrawString(spriteFont, fps, new Vector2(32, 32), Color.White);

            spriteBatch.End();

        }

        SpriteFont spriteFont;
        SpriteBatch spriteBatch;

        float frameCounterStartTimeMillis;
        float frameCounterTimeMillis;
        int frameCount;
        float framePerSec;
    }
}
