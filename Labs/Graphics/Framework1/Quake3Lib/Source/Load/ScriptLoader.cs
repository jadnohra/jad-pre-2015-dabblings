using System;
using System.IO;
using System.Collections.Generic;
using System.Diagnostics;

namespace BlackRice.Framework.Quake3.Load
{
    class ScriptLoader
    {
        internal struct ScriptFile
        {
            internal string assetName;
        }

        internal class ShaderAddress
        {
            internal int file;
            ScriptBlockParser.BlockAddress address;

            internal ShaderAddress(int file_, ScriptBlockParser.BlockAddress address_)
            {
                file = file_;
                address = address_;
            }
        }

        LevelContentManager m_Parent;
        string m_TypeFilter;
        ScriptFile[] m_ScriptFiles;
        Dictionary<string, ShaderAddress> m_ShaderAddresses;

        internal ScriptLoader(LevelContentManager parent, string typeFilter)
        {
            m_Parent = parent;
            m_TypeFilter = typeFilter;

            ParseScripts();
        }

        void ParseScripts()
        {
            if (m_ScriptFiles == null)
            {
                ScriptBlockParser scriptBlockParser = new ScriptBlockParser();

                DirectoryInfo di = new DirectoryInfo(Path.Combine(m_Parent.m_BspLevel.RootPath, "scripts/"));
                FileInfo[] fileInfos = di.GetFiles(m_TypeFilter);

                m_ScriptFiles = new ScriptFile[fileInfos.Length];
                m_ShaderAddresses = new Dictionary<string, ShaderAddress>();

                string scriptName = null;
                ScriptBlockParser.BlockAddress address = new ScriptBlockParser.BlockAddress();


                for (int i = 0; i < fileInfos.Length; ++i)
                {
                    FileInfo fi  = fileInfos[i];
                    m_ScriptFiles[i].assetName = fi.Name;

                    ShaderParser.Scanner scanner = new ShaderParser.Scanner(fi.FullName);
                    ShaderParser.Parser parser = new ShaderParser.Parser(scanner);

                    parser.Parse();

                    if (parser.errors.count > 0)
                    {
                        Trace.TraceWarning("Error parsing '" + fi.FullName + "'");
                    }
                    
                    /*
                    scriptBlockParser.Open(fi.FullName);
                    while (scriptBlockParser.GetNextScriptAddress(ref scriptName, ref address))
                    {
                        m_ShaderAddresses[scriptName] = new ShaderAddress(i, address);
                    }

                    if (scriptBlockParser.HasError())
                        Trace.TraceWarning("Error parsing '" + fi.FullName + "'");
                     */
                }
           }
        }

        public bool HasShaderSource(string assetName)
        {
            if (m_ScriptFiles == null)
                ParseScripts();

            return false;
        }
    }
}