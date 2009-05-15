using System;
using System.Diagnostics;
using System.Collections.Generic;


namespace BlackRice.Framework.Quake3
{
    public struct Interval
    {
        internal int Start;
        internal int End; // inclusive

        public Interval(int start, int end)
        {
            Start = start;
            End = end;
        }

        public bool Contains(int index)
        {
            return index >= Start && index <= End;
        }

        public int Count()
        {
            return (End - Start) + 1;
        }
    };


    public class CoalescedIntervals 
    {
        public Interval[] Intervals;

        public CoalescedIntervals()
        {
        }

        public CoalescedIntervals(Interval[] intervals)
        {
            Intervals = intervals;
        }

        public int Map(int index)
        {
            int gapOffset = 0;

            for (int i = 0; i < Intervals.Length; ++i)
            {
                Interval interval = Intervals[i];

                if (interval.Contains(index))
                {
                    return (index - interval.Start) + gapOffset;
                }

                gapOffset += interval.Count();
            }

            Trace.Assert(false);
            return -1;
        }
    }


    public class IntervalCoalescor : IDisposable
    {
        internal List<Interval> Intervals = new List<Interval>();

        public void AddElement(int index)
        {
            int intervalIndex;

            if (FindIntervalFor(index, out intervalIndex))
            {
                Interval boxed = Intervals[intervalIndex];

                if (boxed.Start > index)
                    boxed.Start = index;
                else if (boxed.End < index)
                    boxed.End = index;

                Intervals[intervalIndex] = boxed;
            }
            else
            {
                Intervals.Add(new Interval(index, index));
            }
        }

        public bool FindIntervalFor(int index, out int i)
        {
            for (i = 0; i < Intervals.Count; ++i)
            {
                if ((Intervals[i].Start - 1) <= index && (Intervals[i].End + 1) >= index)
                {
                    return true;
                }
            }

            return false;
        }

        public int GetElementCount()
        {
            int count = 0;
            foreach (Interval interval in Intervals)
            {
                count += interval.Count();
            }

            return count;
        }

        public static int StartComparison(Interval intrvl, Interval comp)
        {
            int diff = intrvl.Start - comp.Start;

            if (diff == 0)
                diff = comp.End - intrvl.End;

            return diff;
        }

        public void Build()
        {
            Intervals.Sort(StartComparison);
            Coalesce();
        }

        public void BuildCoalescedIntervals(ref CoalescedIntervals intervals)
        {
            intervals.Intervals = Intervals.ToArray();
        }

        void Coalesce()
        {
            for (int i = 0; i + 1 < Intervals.Count; )
            {
                Interval interval = Intervals[i];
                Interval nextInterval = Intervals[i + 1];

                if (Coalesce(ref interval, nextInterval))
                {
                    Intervals[i] = interval;
                    Intervals.RemoveAt(i + 1);
                }
                else
                {
                    ++i;
                }
            }
        }

        bool Coalesce(ref Interval interval, Interval nextInterval)
        {
            Trace.Assert(interval.Start <= nextInterval.Start);

            if (interval.Contains(nextInterval.Start - 1))
            {
                if (nextInterval.End > interval.End)
                    interval.End = nextInterval.End;

                return true;

            }
            else if (interval.Start == nextInterval.Start)
            {
                Trace.Assert(interval.End >= nextInterval.End);

                if (nextInterval.End <= interval.End)
                    return true;
            }

            return false;
        }

        public void Dispose()
        {
            Intervals = null;
        }
    }
}