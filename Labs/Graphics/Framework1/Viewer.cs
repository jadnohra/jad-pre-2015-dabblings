using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.GamerServices;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Media;
using Microsoft.Xna.Framework.Net;
using Microsoft.Xna.Framework.Storage;

namespace Framework1
{
    /// <summary>
    /// This is the main type for your game
    /// </summary>
    public class Viewer : Microsoft.Xna.Framework.Game
    {
        GraphicsDeviceManager graphics;

        Scene m_Scene;
        
        ManualCamera m_Camera;
        Matrix m_Projection;

        Stats m_Stats; 

        public Viewer()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
            this.IsFixedTimeStep = false;
            this.IsMouseVisible = true;
            
            m_Stats = new Stats();
        }

        /// <summary>
        /// Allows the game to perform any initialization it needs to before starting to run.
        /// This is where it can query for any required services and load any non-graphic
        /// related content.  Calling base.Initialize will enumerate through any components
        /// and initialize them as well.
        /// </summary>
        protected override void Initialize()
        {
            // TODO: Add your initialization logic here

            base.Initialize();
        }

        /// <summary>
        /// LoadContent will be called once per game and is the place to load
        /// all of your content.
        /// </summary>
        protected override void LoadContent()
        {
            if (m_Stats != null)
                m_Stats.LoadContent(GraphicsDevice, Content);

            //string filePath = Path.Combine(StorageContainer.TitleLocation, "../../test2.bsp");
            //string filePath = Path.Combine(Content.RootDirectory, "Q3Maps/test1.pk3/maps/test1.bsp");
            string filePath = Path.Combine(Content.RootDirectory, "Q3Maps/jof3dm2.pk3/maps/jof3dm2.bsp");
            //string filePath = Path.Combine(Content.RootDirectory, "Q3Maps/natedm3.pk3/maps/natedm3.bsp");
            //string filePath = Path.Combine(Content.RootDirectory, "Q3Maps/acid3dm9.pk3/maps/acid3dm9.bsp");
            //string filePath = Path.Combine(Content.RootDirectory, "Q3Maps/q3shw23.pk3/maps/q3shw23.bsp");

            using (Quake3.BspFileDumper bspFileDumper = new Quake3.BspFileDumper(filePath))
            {
                bspFileDumper.Print(true);
            }

            {
                Quake3.LevelScene scene = new Quake3.LevelScene();

                if (!scene.Load(filePath, Content, GraphicsDevice))
                {
                    this.Exit();
                }

                m_Scene = scene;
            }

            //m_Renderer = new Renderer(graphics.GraphicsDevice, Content, m_AssetInterpeterFactory);
            m_Camera = new ManualCamera();
            m_Camera.SetMoveScale(170.0f);
            m_Camera.SetNearFarPlanes(5.0f, 5000.0f);
        }

        /// <summary>
        /// UnloadContent will be called once per game and is the place to unload
        /// all content.
        /// </summary>
        protected override void UnloadContent()
        {
            // TODO: Unload any non ContentManager content here

            if (m_Stats != null)
                m_Stats.UnloadContent(GraphicsDevice, Content);
        }

        /// <summary>
        /// Allows the game to run logic such as updating the world,
        /// checking for collisions, gathering input, and playing audio.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Update(GameTime gameTime)
        {
            // Allows the game to exit
            if (GamePad.GetState(PlayerIndex.One).Buttons.Back == ButtonState.Pressed
                || Keyboard.GetState().IsKeyDown(Keys.Escape))
                this.Exit();

            float aspectRatio = (float)graphics.GraphicsDevice.Viewport.Width / (float)graphics.GraphicsDevice.Viewport.Height;
            m_Projection = Matrix.CreatePerspectiveFieldOfView(MathHelper.ToRadians(45.0f), aspectRatio, m_Camera.GetZNear(), m_Camera.GetZFar());

            m_Camera.UpdateInput(GraphicsDevice.Viewport, m_Projection, gameTime);

            if (m_Stats != null)
                m_Stats.Update(gameTime);

            base.Update(gameTime);
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
            if (Keyboard.GetState().IsKeyDown(Keys.F8))
            {
                GraphicsDevice.RenderState.FillMode = FillMode.WireFrame;
            }
            else
            {
                GraphicsDevice.RenderState.FillMode = FillMode.Solid;
            }

            if (Keyboard.GetState().IsKeyDown(Keys.F9))
            {
                GraphicsDevice.RenderState.CullMode = CullMode.None;
            }
            else
            {
                GraphicsDevice.RenderState.CullMode = CullMode.CullCounterClockwiseFace;
            }

            GraphicsDevice.Clear(Color.CornflowerBlue);

            m_Scene.Render(m_Camera, m_Projection);
           // m_Scene.Render(m_Renderer, m_BspHeader, m_Camera.GetViewMatrix(), m_Projection);
            //m_Renderer.Render();

            if (m_Stats != null)
                m_Stats.Draw();

            base.Draw(gameTime);
        }
    }
}
