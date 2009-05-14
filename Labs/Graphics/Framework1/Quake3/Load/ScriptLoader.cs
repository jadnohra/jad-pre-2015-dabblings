using System;
using System.IO;
using System.Collections.Generic;

namespace Framework1.Quake3.Load
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
            internal int pos;
            internal int size;

            internal ShaderAddress(int file_, int pos_, int size_)
            {
                file = file_;
                pos = pos_;
                size = size_;
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

            //ParseScripts();
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
                
                for (int i = 0; i < fileInfos.Length; ++i)
                {
                    FileInfo fi  = fileInfos[i];
                    scriptBlockParser.Open(fi.FullName);
                    m_ScriptFiles[i].assetName = fi.Name;

                    string scriptName = null;
                    int scriptStartPos = 0;
                    int scriptSourceSize = 0;

                    
                    while (scriptBlockParser.GetNextScriptAddress(ref scriptName, ref scriptStartPos, ref scriptSourceSize))
                    {
                        m_ShaderAddresses[scriptName] = new ShaderAddress(i, scriptStartPos, scriptSourceSize);
                    }
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