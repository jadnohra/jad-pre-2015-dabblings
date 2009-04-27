using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content.Pipeline;
using Microsoft.Xna.Framework.Content.Pipeline.Graphics;
using Microsoft.Xna.Framework.Content.Pipeline.Processors;


namespace Framework1.Quake3
{
    public class RAMFile
    {
        byte[] Bytes;

        public RAMFile(string filename)
        {
            Bytes = System.IO.File.ReadAllBytes(filename);
        }
    }

    [ContentImporter(".bsp", DefaultProcessor = "Quake3ContentProcessor",
        DisplayName = "Quake3 Bsp Importer")]
    class BspContentImporter : ContentImporter<RAMFile>
    {
        public override RAMFile Import(string filename, ContentImporterContext context)
        {
            return new RAMFile(filename);
        }
    }

    /// <summary>
    /// This class will be instantiated by the XNA Framework Content Pipeline
    /// to apply custom processing to content data, converting an object of
    /// type TInput to TOutput. The input and output types may be the same if
    /// the processor wishes to alter data without changing its type.
    ///
    /// This should be part of a Content Pipeline Extension Library project.
    ///
    /// TODO: change the ContentProcessor attribute to specify the correct
    /// display name for this processor.
    /// </summary>
    [ContentProcessor(DisplayName = "Quake3ContentProcessor")]
    public class BspContentProcessor : ContentProcessor<RAMFile, RAMFile>
    {
        public override RAMFile Process(RAMFile input, ContentProcessorContext context)
        {
            // TODO: process the input object, and return the modified data.
            return input;
        }
    }
}