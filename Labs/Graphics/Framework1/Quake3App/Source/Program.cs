using System;

namespace Framework1
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        static void Main(string[] args)
        {
            using (UnitTests tests = new UnitTests())
            {
                tests.Main(args);
            }

            using (Viewer viewer = new Viewer())
            {
                viewer.Run();
            }

            /*
            using (Game1 game = new Game1())
            {
                game.Run();
            }
            */
        }
    }
}

