using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Framework1
{
    class Asset
    {
    }

    class AssetPart
    {
    }

    abstract class AssetInterpreter
    {
        public abstract Object Interpret(Asset asset, AssetPart part);
    }

    class AssetInterpeterFactory
    {
        class InterpreterMap
        {
            public Dictionary<Type, AssetInterpreter> Map =
                new Dictionary<Type, AssetInterpreter>();
        }

        Dictionary<Type, InterpreterMap> m_InterepreterMaps =
                new Dictionary<Type, InterpreterMap>();

        public void Insert(Type assetType, Type targetType, AssetInterpreter interpreter)
        {
            InterpreterMap map;

            if (!m_InterepreterMaps.TryGetValue(assetType, out map))
            {
                map = new InterpreterMap();
                m_InterepreterMaps.Add(assetType, map);
            }

            map.Map[targetType] = interpreter;
        }

        public AssetInterpreter GetInterpreter(Type assetType, Type targetType)
        {
            InterpreterMap map;

            if (m_InterepreterMaps.TryGetValue(assetType, out map))
            {
                AssetInterpreter interpreter;

                if (map.Map.TryGetValue(targetType, out interpreter))
                {
                    return interpreter;
                }
            }

            return null;
        }

        public Object Interpret(Asset asset, AssetPart part, Type targetType)
        {
            AssetInterpreter interepreter = GetInterpreter(asset.GetType(), targetType);

            if (interepreter != null)
            {
                return interepreter.Interpret(asset, part);
            }

            return null;
        }
    }
}
