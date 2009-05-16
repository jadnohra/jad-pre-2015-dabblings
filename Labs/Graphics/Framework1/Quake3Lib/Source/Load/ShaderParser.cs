
using System;

namespace BlackRice.Framework.Quake3.ShaderParser {



public class Parser {
	public const int _EOF = 0;
	public const int _path = 1;
	public const int _number = 2;
	public const int maxT = 209;

	const bool T = true;
	const bool x = false;
	const int minErrDist = 2;
	
	public Scanner scanner;
	public Errors  errors;

	public Token t;    // last recognized token
	public Token la;   // lookahead token
	int errDist = minErrDist;



	public Parser(Scanner scanner) {
		this.scanner = scanner;
		errors = new Errors();
	}

	void SynErr (int n) {
		if (errDist >= minErrDist) errors.SynErr(la.line, la.col, n);
		errDist = 0;
	}

	public void SemErr (string msg) {
		if (errDist >= minErrDist) errors.SemErr(t.line, t.col, msg);
		errDist = 0;
	}
	
	void Get () {
		for (;;) {
			t = la;
			la = scanner.Scan();
			if (la.kind <= maxT) { ++errDist; break; }

			la = t;
		}
	}
	
	void Expect (int n) {
		if (la.kind==n) Get(); else { SynErr(n); }
	}
	
	bool StartOf (int s) {
		return set[s, la.kind];
	}
	
	void ExpectWeak (int n, int follow) {
		if (la.kind == n) Get();
		else {
			SynErr(n);
			while (!StartOf(follow)) Get();
		}
	}


	bool WeakSeparator(int n, int syFol, int repFol) {
		int kind = la.kind;
		if (kind == n) {Get(); return true;}
		else if (StartOf(repFol)) {return false;}
		else {
			SynErr(n);
			while (!(set[syFol, kind] || set[repFol, kind] || set[0, kind])) {
				Get();
				kind = la.kind;
			}
			return StartOf(syFol);
		}
	}

	
	void Path() {
		Expect(1);
	}

	void Number() {
		Expect(2);
	}

	void GeneratorFunctionType() {
		switch (la.kind) {
		case 3: {
			Get();
			break;
		}
		case 4: {
			Get();
			break;
		}
		case 5: {
			Get();
			break;
		}
		case 6: {
			Get();
			break;
		}
		case 7: {
			Get();
			break;
		}
		case 8: {
			Get();
			break;
		}
		case 9: {
			Get();
			break;
		}
		default: SynErr(210); break;
		}
	}

	void GeneratorFunction() {
		GeneratorFunctionType();
		Number();
		Number();
		Number();
		Number();
	}

	void Q3Map2Func() {
		switch (la.kind) {
		case 10: {
			Get();
			Expect(11);
			Number();
			Number();
			Number();
			Expect(12);
			break;
		}
		case 13: {
			Get();
			Expect(11);
			Number();
			Number();
			Number();
			Expect(12);
			break;
		}
		case 14: {
			Get();
			Expect(11);
			Number();
			Number();
			Number();
			Number();
			Number();
			Expect(12);
			break;
		}
		case 15: {
			Get();
			Expect(11);
			Number();
			Number();
			Number();
			Number();
			Number();
			Expect(12);
			break;
		}
		case 16: {
			Get();
			Number();
			break;
		}
		case 17: {
			Get();
			Number();
			break;
		}
		default: SynErr(211); break;
		}
	}

	void Q3Map2TcGenFunc() {
		if (la.kind == 18) {
			Get();
			Number();
			Number();
		} else if (la.kind == 19) {
			Get();
			Number();
			Number();
		} else SynErr(212);
	}

	void Q3Map2TcModFunc() {
		if (la.kind == 20) {
			Get();
			Number();
		} else if (la.kind == 16) {
			Get();
			Number();
			Number();
		} else if (la.kind == 21 || la.kind == 22 || la.kind == 23) {
			if (la.kind == 21) {
				Get();
			} else if (la.kind == 22) {
				Get();
			} else {
				Get();
			}
			Number();
			Number();
		} else SynErr(213);
	}

	void BlendMode() {
		switch (la.kind) {
		case 24: {
			Get();
			break;
		}
		case 25: {
			Get();
			break;
		}
		case 26: {
			Get();
			break;
		}
		case 27: {
			Get();
			break;
		}
		case 28: {
			Get();
			break;
		}
		case 29: {
			Get();
			break;
		}
		case 30: {
			Get();
			break;
		}
		case 31: {
			Get();
			break;
		}
		case 32: {
			Get();
			break;
		}
		case 33: {
			Get();
			break;
		}
		default: SynErr(214); break;
		}
	}

	void RgbGen() {
		Expect(34);
		switch (la.kind) {
		case 35: {
			Get();
			break;
		}
		case 36: {
			Get();
			break;
		}
		case 37: {
			Get();
			break;
		}
		case 38: {
			Get();
			break;
		}
		case 39: {
			Get();
			break;
		}
		case 40: {
			Get();
			break;
		}
		case 41: {
			Get();
			break;
		}
		case 42: {
			Get();
			break;
		}
		case 43: {
			Get();
			break;
		}
		case 44: {
			Get();
			GeneratorFunction();
			break;
		}
		case 45: {
			Get();
			Expect(11);
			Number();
			Number();
			Number();
			Expect(12);
			break;
		}
		default: SynErr(215); break;
		}
	}

	void AlphaGen() {
		Expect(46);
		switch (la.kind) {
		case 35: {
			Get();
			break;
		}
		case 36: {
			Get();
			break;
		}
		case 37: {
			Get();
			break;
		}
		case 38: {
			Get();
			break;
		}
		case 39: {
			Get();
			break;
		}
		case 40: {
			Get();
			break;
		}
		case 41: {
			Get();
			break;
		}
		case 42: {
			Get();
			break;
		}
		case 43: {
			Get();
			break;
		}
		case 44: {
			Get();
			GeneratorFunction();
			break;
		}
		case 45: {
			Get();
			Number();
			break;
		}
		case 47: {
			Get();
			Number();
			break;
		}
		default: SynErr(216); break;
		}
	}

	void TcGen() {
		Expect(48);
		if (la.kind == 49) {
			Get();
		} else if (la.kind == 50) {
			Get();
		} else if (la.kind == 51) {
			Get();
		} else if (la.kind == 18) {
			Get();
			Expect(11);
			Number();
			Number();
			Number();
			Expect(12);
			Expect(11);
			Number();
			Number();
			Number();
			Expect(12);
		} else SynErr(217);
	}

	void TcMod() {
		Expect(52);
		switch (la.kind) {
		case 20: {
			Get();
			Number();
			break;
		}
		case 16: {
			Get();
			Number();
			Number();
			break;
		}
		case 53: {
			Get();
			Number();
			Number();
			break;
		}
		case 54: {
			Get();
			GeneratorFunction();
			break;
		}
		case 55: {
			Get();
			Number();
			Number();
			Number();
			Number();
			Number();
			Number();
			break;
		}
		case 56: {
			Get();
			Number();
			Number();
			Number();
			Number();
			break;
		}
		case 57: {
			Get();
			break;
		}
		default: SynErr(218); break;
		}
	}

	void AlphaFunc() {
		Expect(58);
		if (la.kind == 59) {
			Get();
		} else if (la.kind == 60) {
			Get();
		} else if (la.kind == 61) {
			Get();
		} else SynErr(219);
	}

	void Stage() {
		Expect(62);
		while (StartOf(1)) {
			switch (la.kind) {
			case 63: {
				Get();
				if (la.kind == 64) {
					Get();
				} else if (la.kind == 65 || la.kind == 66) {
					if (la.kind == 65) {
						Get();
					} else {
						Get();
					}
				} else if (la.kind == 1) {
					Path();
				} else SynErr(220);
				break;
			}
			case 67: {
				Get();
				Path();
				break;
			}
			case 68: {
				Get();
				Number();
				Path();
				while (la.kind == 1) {
					Path();
				}
				break;
			}
			case 69: {
				Get();
				if (la.kind == 70) {
					Get();
				} else if (la.kind == 71) {
					Get();
				} else if (la.kind == 72) {
					Get();
				} else if (StartOf(2)) {
					BlendMode();
					BlendMode();
				} else SynErr(221);
				break;
			}
			case 34: {
				RgbGen();
				break;
			}
			case 46: {
				AlphaGen();
				break;
			}
			case 48: {
				TcGen();
				break;
			}
			case 52: {
				TcMod();
				break;
			}
			case 58: {
				AlphaFunc();
				break;
			}
			case 73: {
				Get();
				if (la.kind == 74) {
					Get();
				} else if (la.kind == 75) {
					Get();
				} else if (la.kind == 76) {
					Get();
				} else SynErr(222);
				break;
			}
			case 77: {
				Get();
				break;
			}
			case 78: {
				Get();
				break;
			}
			}
		}
		Expect(79);
	}

	void SkyParms() {
		Expect(80);
		if (la.kind == 1) {
			Path();
		} else if (la.kind == 81) {
			Get();
		} else SynErr(223);
		Number();
		if (la.kind == 1) {
			Path();
		} else if (la.kind == 81) {
			Get();
		} else SynErr(224);
	}

	void DeformVertexes() {
		Expect(82);
		switch (la.kind) {
		case 44: {
			Get();
			Number();
			if (StartOf(3)) {
				GeneratorFunction();
			}
			break;
		}
		case 83: {
			Get();
			Number();
			if (la.kind == 2) {
				Number();
			} else if (StartOf(3)) {
				GeneratorFunction();
			} else SynErr(225);
			break;
		}
		case 23: {
			Get();
			Number();
			Number();
			Number();
			GeneratorFunction();
			break;
		}
		case 84: {
			Get();
			Number();
			Number();
			Number();
			break;
		}
		case 85: {
			Get();
			break;
		}
		case 86: {
			Get();
			break;
		}
		case 87: {
			Get();
			break;
		}
		default: SynErr(226); break;
		}
	}

	void FogParms() {
		Expect(88);
		Expect(11);
		Number();
		Number();
		Number();
		Expect(12);
		Number();
	}

	void SurfaceParm() {
		Expect(89);
		switch (la.kind) {
		case 90: {
			Get();
			break;
		}
		case 91: {
			Get();
			break;
		}
		case 92: {
			Get();
			break;
		}
		case 93: {
			Get();
			break;
		}
		case 94: {
			Get();
			break;
		}
		case 95: {
			Get();
			break;
		}
		case 96: {
			Get();
			break;
		}
		case 97: {
			Get();
			break;
		}
		case 98: {
			Get();
			break;
		}
		case 99: {
			Get();
			break;
		}
		case 100: {
			Get();
			break;
		}
		case 101: {
			Get();
			break;
		}
		case 102: {
			Get();
			break;
		}
		case 103: {
			Get();
			break;
		}
		case 104: {
			Get();
			break;
		}
		case 105: {
			Get();
			break;
		}
		case 106: {
			Get();
			break;
		}
		case 107: {
			Get();
			break;
		}
		case 108: {
			Get();
			break;
		}
		case 109: {
			Get();
			break;
		}
		case 110: {
			Get();
			break;
		}
		case 111: {
			Get();
			break;
		}
		case 112: {
			Get();
			break;
		}
		case 113: {
			Get();
			break;
		}
		case 114: {
			Get();
			break;
		}
		case 115: {
			Get();
			break;
		}
		case 116: {
			Get();
			break;
		}
		case 117: {
			Get();
			break;
		}
		case 118: {
			Get();
			break;
		}
		case 119: {
			Get();
			break;
		}
		case 120: {
			Get();
			break;
		}
		case 78: {
			Get();
			break;
		}
		case 121: {
			Get();
			break;
		}
		case 122: {
			Get();
			break;
		}
		case 123: {
			Get();
			break;
		}
		case 124: {
			Get();
			break;
		}
		default: SynErr(227); break;
		}
	}

	void ShaderDecl() {
		Path();
		Expect(62);
		while (StartOf(4)) {
			switch (la.kind) {
			case 80: {
				SkyParms();
				break;
			}
			case 62: {
				Stage();
				break;
			}
			case 82: {
				DeformVertexes();
				break;
			}
			case 88: {
				FogParms();
				break;
			}
			case 125: {
				Get();
				if (la.kind == 126) {
					Get();
				} else if (la.kind == 127) {
					Get();
				} else if (la.kind == 76) {
					Get();
				} else if (la.kind == 128) {
					Get();
				} else SynErr(228);
				break;
			}
			case 129: {
				Get();
				switch (la.kind) {
				case 2: {
					Number();
					break;
				}
				case 47: {
					Get();
					break;
				}
				case 117: {
					Get();
					break;
				}
				case 130: {
					Get();
					break;
				}
				case 131: {
					Get();
					break;
				}
				case 132: {
					Get();
					break;
				}
				case 133: {
					Get();
					break;
				}
				case 134: {
					Get();
					break;
				}
				default: SynErr(229); break;
				}
				break;
			}
			case 135: {
				Get();
				break;
			}
			case 136: {
				Get();
				break;
			}
			case 137: {
				Get();
				break;
			}
			case 47: {
				Get();
				break;
			}
			case 138: {
				Get();
				break;
			}
			case 139: {
				Get();
				Number();
				break;
			}
			case 140: {
				Get();
				Number();
				break;
			}
			case 89: {
				SurfaceParm();
				break;
			}
			case 141: {
				Get();
				Path();
				break;
			}
			case 142: {
				Get();
				break;
			}
			case 143: {
				Get();
				Number();
				break;
			}
			case 144: {
				Get();
				Q3Map2Func();
				break;
			}
			case 145: {
				Get();
				Q3Map2Func();
				break;
			}
			case 146: {
				Get();
				Path();
				break;
			}
			case 147: {
				Get();
				Number();
				Number();
				break;
			}
			case 148: {
				Get();
				Path();
				break;
			}
			case 149: {
				Get();
				Number();
				break;
			}
			case 150: {
				Get();
				Number();
				break;
			}
			case 151: {
				Get();
				break;
			}
			case 152: {
				Get();
				Path();
				break;
			}
			case 153: {
				Get();
				break;
			}
			case 154: {
				Get();
				break;
			}
			case 155: {
				Get();
				Number();
				break;
			}
			case 156: {
				Get();
				break;
			}
			case 157: {
				Get();
				break;
			}
			case 158: {
				Get();
				Number();
				Number();
				Number();
				break;
			}
			case 159: {
				Get();
				break;
			}
			case 160: {
				Get();
				break;
			}
			case 161: {
				Get();
				break;
			}
			case 162: {
				Get();
				Path();
				break;
			}
			case 163: {
				Get();
				Number();
				break;
			}
			case 164: {
				Get();
				Number();
				break;
			}
			case 165: {
				Get();
				Number();
				Number();
				break;
			}
			case 166: {
				Get();
				Number();
				break;
			}
			case 167: {
				Get();
				break;
			}
			case 168: {
				Get();
				Number();
				break;
			}
			case 169: {
				Get();
				Number();
				break;
			}
			case 170: {
				Get();
				Number();
				Number();
				break;
			}
			case 171: {
				Get();
				Number();
				Number();
				Number();
				break;
			}
			case 172: {
				Get();
				Number();
				break;
			}
			case 173: {
				Get();
				Number();
				break;
			}
			case 174: {
				Get();
				break;
			}
			case 175: {
				Get();
				break;
			}
			case 176: {
				Get();
				break;
			}
			case 177: {
				Get();
				break;
			}
			case 178: {
				Get();
				Path();
				break;
			}
			case 179: {
				Get();
				break;
			}
			case 180: {
				Get();
				break;
			}
			case 181: {
				Get();
				break;
			}
			case 182: {
				Get();
				Number();
				break;
			}
			case 183: {
				Get();
				break;
			}
			case 184: {
				Get();
				Number();
				break;
			}
			case 185: {
				Get();
				break;
			}
			case 186: {
				Get();
				break;
			}
			case 187: {
				Get();
				break;
			}
			case 188: {
				Get();
				Number();
				break;
			}
			case 189: {
				Get();
				Number();
				Number();
				break;
			}
			case 190: {
				Get();
				break;
			}
			case 191: {
				Get();
				break;
			}
			case 192: {
				Get();
				break;
			}
			case 193: {
				Get();
				Number();
				Number();
				Number();
				Number();
				Number();
				Number();
				break;
			}
			case 194: {
				Get();
				Number();
				Number();
				Number();
				Number();
				Number();
				Number();
				Number();
				Number();
				break;
			}
			case 195: {
				Get();
				break;
			}
			case 196: {
				Get();
				if (la.kind == 2) {
					Number();
				} else if (la.kind == 1) {
					Path();
				} else SynErr(230);
				break;
			}
			case 197: {
				Get();
				Path();
				Number();
				Number();
				Number();
				Number();
				Number();
				Number();
				Number();
				break;
			}
			case 198: {
				Get();
				Q3Map2TcGenFunc();
				break;
			}
			case 199: {
				Get();
				Q3Map2TcModFunc();
				break;
			}
			case 200: {
				Get();
				break;
			}
			case 201: {
				Get();
				Number();
				break;
			}
			case 202: {
				Get();
				Number();
				Number();
				break;
			}
			case 203: {
				Get();
				break;
			}
			case 204: {
				Get();
				Number();
				break;
			}
			case 205: {
				Get();
				break;
			}
			case 206: {
				Get();
				break;
			}
			case 207: {
				Get();
				Path();
				break;
			}
			case 208: {
				Get();
				break;
			}
			}
		}
		Expect(79);
	}

	void Q3Shader() {
		while (la.kind == 1) {
			ShaderDecl();
		}
	}



	public void Parse() {
		la = new Token();
		la.val = "";		
		Get();
		Q3Shader();

    Expect(0);
	}
	
	static readonly bool[,] set = {
		{T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,x, x,x,x,x, x,x,x,x, x,x,T,x, T,x,x,x, T,x,x,x, x,x,T,x, x,x,x,T, x,x,x,T, T,T,x,x, x,T,x,x, x,T,T,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, T,T,T,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x},
		{x,x,x,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,T, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, T,x,T,x, x,x,x,x, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,T,x,x, x,T,x,x, x,x,x,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,x,x}

	};
} // end Parser


public class Errors {
	public int count = 0;                                    // number of errors detected
	public System.IO.TextWriter errorStream = Console.Out;   // error messages go to this stream
  public string errMsgFormat = "-- line {0} col {1}: {2}"; // 0=line, 1=column, 2=text
  
	public void SynErr (int line, int col, int n) {
		string s;
		switch (n) {
			case 0: s = "EOF expected"; break;
			case 1: s = "path expected"; break;
			case 2: s = "number expected"; break;
			case 3: s = "\"sin\" expected"; break;
			case 4: s = "\"triangle\" expected"; break;
			case 5: s = "\"square\" expected"; break;
			case 6: s = "\"sawtooth\" expected"; break;
			case 7: s = "\"inversesawtooth\" expected"; break;
			case 8: s = "\"random\" expected"; break;
			case 9: s = "\"noise\" expected"; break;
			case 10: s = "\"dotproduct\" expected"; break;
			case 11: s = "\"(\" expected"; break;
			case 12: s = "\")\" expected"; break;
			case 13: s = "\"dotproduct2\" expected"; break;
			case 14: s = "\"dotproductscale\" expected"; break;
			case 15: s = "\"dotproduct2scale\" expected"; break;
			case 16: s = "\"scale\" expected"; break;
			case 17: s = "\"set\" expected"; break;
			case 18: s = "\"vector\" expected"; break;
			case 19: s = "\"ivector\" expected"; break;
			case 20: s = "\"rotate\" expected"; break;
			case 21: s = "\"translate\" expected"; break;
			case 22: s = "\"shift\" expected"; break;
			case 23: s = "\"move\" expected"; break;
			case 24: s = "\"gl_zero\" expected"; break;
			case 25: s = "\"gl_one\" expected"; break;
			case 26: s = "\"gl_src_color\" expected"; break;
			case 27: s = "\"gl_one_minus_src_color\" expected"; break;
			case 28: s = "\"gl_dst_color\" expected"; break;
			case 29: s = "\"gl_one_minus_dst_color\" expected"; break;
			case 30: s = "\"gl_src_alpha\" expected"; break;
			case 31: s = "\"gl_one_minus_src_alpha\" expected"; break;
			case 32: s = "\"gl_dst_alpha\" expected"; break;
			case 33: s = "\"gl_one_minus_dst_alpha\" expected"; break;
			case 34: s = "\"rgbgen\" expected"; break;
			case 35: s = "\"identitylighting\" expected"; break;
			case 36: s = "\"identity\" expected"; break;
			case 37: s = "\"entity\" expected"; break;
			case 38: s = "\"oneminusentity\" expected"; break;
			case 39: s = "\"vertex\" expected"; break;
			case 40: s = "\"oneminusvertex\" expected"; break;
			case 41: s = "\"exactvertex\" expected"; break;
			case 42: s = "\"lightingdiffuse\" expected"; break;
			case 43: s = "\"lightingspecular\" expected"; break;
			case 44: s = "\"wave\" expected"; break;
			case 45: s = "\"const\" expected"; break;
			case 46: s = "\"alphagen\" expected"; break;
			case 47: s = "\"portal\" expected"; break;
			case 48: s = "\"tcgen\" expected"; break;
			case 49: s = "\"base\" expected"; break;
			case 50: s = "\"lightmap\" expected"; break;
			case 51: s = "\"environment\" expected"; break;
			case 52: s = "\"tcmod\" expected"; break;
			case 53: s = "\"scroll\" expected"; break;
			case 54: s = "\"stretch\" expected"; break;
			case 55: s = "\"transform\" expected"; break;
			case 56: s = "\"turb\" expected"; break;
			case 57: s = "\"entitytranslate\" expected"; break;
			case 58: s = "\"alphafunc\" expected"; break;
			case 59: s = "\"gt0\" expected"; break;
			case 60: s = "\"lt128\" expected"; break;
			case 61: s = "\"ge128\" expected"; break;
			case 62: s = "\"{\" expected"; break;
			case 63: s = "\"map\" expected"; break;
			case 64: s = "\"$lightmap\" expected"; break;
			case 65: s = "\"$whiteimage\" expected"; break;
			case 66: s = "\"*white\" expected"; break;
			case 67: s = "\"clampmap\" expected"; break;
			case 68: s = "\"animmap\" expected"; break;
			case 69: s = "\"blendfunc\" expected"; break;
			case 70: s = "\"add\" expected"; break;
			case 71: s = "\"filter\" expected"; break;
			case 72: s = "\"blend\" expected"; break;
			case 73: s = "\"depthfunc\" expected"; break;
			case 74: s = "\"equal\" expected"; break;
			case 75: s = "\"lequal\" expected"; break;
			case 76: s = "\"disable\" expected"; break;
			case 77: s = "\"depthwrite\" expected"; break;
			case 78: s = "\"detail\" expected"; break;
			case 79: s = "\"}\" expected"; break;
			case 80: s = "\"skyparms\" expected"; break;
			case 81: s = "\"-\" expected"; break;
			case 82: s = "\"deformvertexes\" expected"; break;
			case 83: s = "\"normal\" expected"; break;
			case 84: s = "\"bulge\" expected"; break;
			case 85: s = "\"autosprite\" expected"; break;
			case 86: s = "\"autosprite2\" expected"; break;
			case 87: s = "\"projectionshadow\" expected"; break;
			case 88: s = "\"fogparms\" expected"; break;
			case 89: s = "\"surfaceparm\" expected"; break;
			case 90: s = "\"alphashadow\" expected"; break;
			case 91: s = "\"areaportal\" expected"; break;
			case 92: s = "\"clusterportal\" expected"; break;
			case 93: s = "\"donotenter\" expected"; break;
			case 94: s = "\"flesh\" expected"; break;
			case 95: s = "\"fog\" expected"; break;
			case 96: s = "\"lava\" expected"; break;
			case 97: s = "\"metalsteps\" expected"; break;
			case 98: s = "\"nodamage\" expected"; break;
			case 99: s = "\"nodlight\" expected"; break;
			case 100: s = "\"nodraw\" expected"; break;
			case 101: s = "\"nodrop\" expected"; break;
			case 102: s = "\"noimpact\" expected"; break;
			case 103: s = "\"nomarks\" expected"; break;
			case 104: s = "\"nolightmap\" expected"; break;
			case 105: s = "\"nosteps\" expected"; break;
			case 106: s = "\"nonsolid\" expected"; break;
			case 107: s = "\"origin\" expected"; break;
			case 108: s = "\"playerclip\" expected"; break;
			case 109: s = "\"slick\" expected"; break;
			case 110: s = "\"slime\" expected"; break;
			case 111: s = "\"structural\" expected"; break;
			case 112: s = "\"trans\" expected"; break;
			case 113: s = "\"water\" expected"; break;
			case 114: s = "\"pointlight\" expected"; break;
			case 115: s = "\"forcefield\" expected"; break;
			case 116: s = "\"shotclip\" expected"; break;
			case 117: s = "\"sky\" expected"; break;
			case 118: s = "\"monsterclip\" expected"; break;
			case 119: s = "\"hint\" expected"; break;
			case 120: s = "\"ladder\" expected"; break;
			case 121: s = "\"dust\" expected"; break;
			case 122: s = "\"botclip\" expected"; break;
			case 123: s = "\"antiportal\" expected"; break;
			case 124: s = "\"lightgrid\" expected"; break;
			case 125: s = "\"cull\" expected"; break;
			case 126: s = "\"front\" expected"; break;
			case 127: s = "\"back\" expected"; break;
			case 128: s = "\"none\" expected"; break;
			case 129: s = "\"sort\" expected"; break;
			case 130: s = "\"opaque\" expected"; break;
			case 131: s = "\"banner\" expected"; break;
			case 132: s = "\"underwater\" expected"; break;
			case 133: s = "\"additive\" expected"; break;
			case 134: s = "\"nearest\" expected"; break;
			case 135: s = "\"nopicmip\" expected"; break;
			case 136: s = "\"nomipmaps\" expected"; break;
			case 137: s = "\"polygonoffset\" expected"; break;
			case 138: s = "\"entitymergable\" expected"; break;
			case 139: s = "\"tesssize\" expected"; break;
			case 140: s = "\"light\" expected"; break;
			case 141: s = "\"qer_editorimage\" expected"; break;
			case 142: s = "\"qer_nocarve\" expected"; break;
			case 143: s = "\"qer_trans\" expected"; break;
			case 144: s = "\"q3map_alphagen\" expected"; break;
			case 145: s = "\"q3map_alphamod\" expected"; break;
			case 146: s = "\"q3map_backshader\" expected"; break;
			case 147: s = "\"q3map_backsplash\" expected"; break;
			case 148: s = "\"q3map_baseshader\" expected"; break;
			case 149: s = "\"q3map_bounce\" expected"; break;
			case 150: s = "\"q3map_bouncescale\" expected"; break;
			case 151: s = "\"q3map_clipmodel\" expected"; break;
			case 152: s = "\"q3map_cloneshader\" expected"; break;
			case 153: s = "\"q3map_colorgen\" expected"; break;
			case 154: s = "\"q3map_colormod\" expected"; break;
			case 155: s = "\"q3map_fogdir\" expected"; break;
			case 156: s = "\"q3map_forcemeta\" expected"; break;
			case 157: s = "\"q3map_forcesunlight\" expected"; break;
			case 158: s = "\"q3map_fur\" expected"; break;
			case 159: s = "\"q3map_globaltexture\" expected"; break;
			case 160: s = "\"q3map_indexed\" expected"; break;
			case 161: s = "\"q3map_invert\" expected"; break;
			case 162: s = "\"q3map_lightimage\" expected"; break;
			case 163: s = "\"q3map_lightmapaxis\" expected"; break;
			case 164: s = "\"q3map_lightmapbrightness\" expected"; break;
			case 165: s = "\"q3map_lightmapfilterradius\" expected"; break;
			case 166: s = "\"q3map_lightmapgamma\" expected"; break;
			case 167: s = "\"q3map_lightmapmergable\" expected"; break;
			case 168: s = "\"q3map_lightmapsampleoffset\" expected"; break;
			case 169: s = "\"q3map_lightmapsamplesize\" expected"; break;
			case 170: s = "\"q3map_lightmapsize\" expected"; break;
			case 171: s = "\"q3map_lightrgb\" expected"; break;
			case 172: s = "\"q3map_lightstyle\" expected"; break;
			case 173: s = "\"q3map_lightsubdivide\" expected"; break;
			case 174: s = "\"q3map_noclip\" expected"; break;
			case 175: s = "\"q3map_nofast\" expected"; break;
			case 176: s = "\"q3map_nofog\" expected"; break;
			case 177: s = "\"q3map_nonplanar\" expected"; break;
			case 178: s = "\"q3map_normalimage\" expected"; break;
			case 179: s = "\"q3map_notjunc\" expected"; break;
			case 180: s = "\"q3map_novertexlight\" expected"; break;
			case 181: s = "\"q3map_novertexshadows\" expected"; break;
			case 182: s = "\"q3map_offset\" expected"; break;
			case 183: s = "\"q3map_patchshadows\" expected"; break;
			case 184: s = "\"q3map_remapshader\" expected"; break;
			case 185: s = "\"q3map_replicate\" expected"; break;
			case 186: s = "\"q3map_rgbgen\" expected"; break;
			case 187: s = "\"q3map_rgbmod\" expected"; break;
			case 188: s = "\"q3map_shadeangle\" expected"; break;
			case 189: s = "\"q3map_skylight\" expected"; break;
			case 190: s = "\"q3map_splotchfix\" expected"; break;
			case 191: s = "\"q3map_stylemarker\" expected"; break;
			case 192: s = "\"q3map_stylemarker2\" expected"; break;
			case 193: s = "\"q3map_sun\" expected"; break;
			case 194: s = "\"q3map_sunext\" expected"; break;
			case 195: s = "\"q3map_sunlight\" expected"; break;
			case 196: s = "\"q3map_surfacelight\" expected"; break;
			case 197: s = "\"q3map_surfacemodel\" expected"; break;
			case 198: s = "\"q3map_tcgen\" expected"; break;
			case 199: s = "\"q3map_tcmod\" expected"; break;
			case 200: s = "\"q3map_terrain\" expected"; break;
			case 201: s = "\"q3map_tesssize\" expected"; break;
			case 202: s = "\"q3map_texturesize\" expected"; break;
			case 203: s = "\"q3map_tracelight\" expected"; break;
			case 204: s = "\"q3map_vertexscale\" expected"; break;
			case 205: s = "\"q3map_vertexshadows\" expected"; break;
			case 206: s = "\"q3map_vlight\" expected"; break;
			case 207: s = "\"q3map_flare\" expected"; break;
			case 208: s = "\"q3map_nolightmap\" expected"; break;
			case 209: s = "??? expected"; break;
			case 210: s = "invalid GeneratorFunctionType"; break;
			case 211: s = "invalid Q3Map2Func"; break;
			case 212: s = "invalid Q3Map2TcGenFunc"; break;
			case 213: s = "invalid Q3Map2TcModFunc"; break;
			case 214: s = "invalid BlendMode"; break;
			case 215: s = "invalid RgbGen"; break;
			case 216: s = "invalid AlphaGen"; break;
			case 217: s = "invalid TcGen"; break;
			case 218: s = "invalid TcMod"; break;
			case 219: s = "invalid AlphaFunc"; break;
			case 220: s = "invalid Stage"; break;
			case 221: s = "invalid Stage"; break;
			case 222: s = "invalid Stage"; break;
			case 223: s = "invalid SkyParms"; break;
			case 224: s = "invalid SkyParms"; break;
			case 225: s = "invalid DeformVertexes"; break;
			case 226: s = "invalid DeformVertexes"; break;
			case 227: s = "invalid SurfaceParm"; break;
			case 228: s = "invalid ShaderDecl"; break;
			case 229: s = "invalid ShaderDecl"; break;
			case 230: s = "invalid ShaderDecl"; break;

			default: s = "error " + n; break;
		}
		errorStream.WriteLine(errMsgFormat, line, col, s);
		count++;
	}

	public void SemErr (int line, int col, string s) {
		errorStream.WriteLine(errMsgFormat, line, col, s);
		count++;
	}
	
	public void SemErr (string s) {
		errorStream.WriteLine(s);
		count++;
	}
	
	public void Warning (int line, int col, string s) {
		errorStream.WriteLine(errMsgFormat, line, col, s);
	}
	
	public void Warning(string s) {
		errorStream.WriteLine(s);
	}
} // Errors


public class FatalError: Exception {
	public FatalError(string m): base(m) {}
}

}