
using System;

namespace BlackRice.Framework.Quake3.ShaderParser {



public class Parser {
	public const int _EOF = 0;
	public const int _path = 1;
	public const int _number = 2;
	public const int maxT = 141;

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

	void GeneratorFunction() {
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
		default: SynErr(142); break;
		}
	}

	void BlendMode() {
		switch (la.kind) {
		case 10: {
			Get();
			break;
		}
		case 11: {
			Get();
			break;
		}
		case 12: {
			Get();
			break;
		}
		case 13: {
			Get();
			break;
		}
		case 14: {
			Get();
			break;
		}
		case 15: {
			Get();
			break;
		}
		case 16: {
			Get();
			break;
		}
		case 17: {
			Get();
			break;
		}
		case 18: {
			Get();
			break;
		}
		case 19: {
			Get();
			break;
		}
		default: SynErr(143); break;
		}
	}

	void RgbGen() {
		Expect(20);
		switch (la.kind) {
		case 21: {
			Get();
			break;
		}
		case 22: {
			Get();
			break;
		}
		case 23: {
			Get();
			break;
		}
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
		case 28: case 29: {
			if (la.kind == 28) {
				Get();
			} else {
				Get();
			}
			break;
		}
		case 30: {
			Get();
			break;
		}
		case 31: {
			Get();
			GeneratorFunction();
			Number();
			Number();
			Number();
			Number();
			break;
		}
		case 32: {
			Get();
			Expect(33);
			Number();
			Number();
			Number();
			Expect(34);
			break;
		}
		default: SynErr(144); break;
		}
	}

	void AlphaGen() {
		Expect(35);
		switch (la.kind) {
		case 21: {
			Get();
			break;
		}
		case 22: {
			Get();
			break;
		}
		case 23: {
			Get();
			break;
		}
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
			GeneratorFunction();
			Number();
			Number();
			Number();
			Number();
			break;
		}
		case 32: {
			Get();
			Number();
			break;
		}
		case 36: {
			Get();
			Number();
			break;
		}
		default: SynErr(145); break;
		}
	}

	void TcGen() {
		Expect(37);
		if (la.kind == 38) {
			Get();
		} else if (la.kind == 39) {
			Get();
		} else if (la.kind == 40) {
			Get();
		} else if (la.kind == 41) {
			Get();
			Expect(33);
			Number();
			Number();
			Number();
			Expect(34);
			Expect(33);
			Number();
			Number();
			Number();
			Expect(34);
		} else SynErr(146);
	}

	void TcMod() {
		Expect(42);
		switch (la.kind) {
		case 43: {
			Get();
			Number();
			break;
		}
		case 44: {
			Get();
			Number();
			Number();
			break;
		}
		case 45: {
			Get();
			Number();
			Number();
			break;
		}
		case 46: {
			Get();
			GeneratorFunction();
			Number();
			Number();
			Number();
			Number();
			break;
		}
		case 47: {
			Get();
			Number();
			Number();
			Number();
			Number();
			Number();
			Number();
			break;
		}
		case 48: {
			Get();
			Number();
			Number();
			Number();
			Number();
			break;
		}
		case 49: {
			Get();
			break;
		}
		default: SynErr(147); break;
		}
	}

	void AlphaFunc() {
		Expect(50);
		if (la.kind == 51) {
			Get();
		} else if (la.kind == 52) {
			Get();
		} else if (la.kind == 53) {
			Get();
		} else SynErr(148);
	}

	void Stage() {
		Expect(54);
		while (StartOf(1)) {
			switch (la.kind) {
			case 55: {
				Get();
				if (la.kind == 56) {
					Get();
				} else if (la.kind == 57 || la.kind == 58) {
					if (la.kind == 57) {
						Get();
					} else {
						Get();
					}
				} else if (la.kind == 1) {
					Path();
				} else SynErr(149);
				break;
			}
			case 59: {
				Get();
				Path();
				break;
			}
			case 60: {
				Get();
				Number();
				Path();
				while (la.kind == 1) {
					Path();
				}
				break;
			}
			case 61: {
				Get();
				if (la.kind == 62) {
					Get();
				} else if (la.kind == 63) {
					Get();
				} else if (la.kind == 64) {
					Get();
				} else if (StartOf(2)) {
					BlendMode();
					BlendMode();
				} else SynErr(150);
				break;
			}
			case 20: {
				RgbGen();
				break;
			}
			case 35: {
				AlphaGen();
				break;
			}
			case 37: {
				TcGen();
				break;
			}
			case 42: {
				TcMod();
				break;
			}
			case 50: {
				AlphaFunc();
				break;
			}
			case 65: {
				Get();
				if (la.kind == 66) {
					Get();
				} else if (la.kind == 67) {
					Get();
				} else if (la.kind == 68) {
					Get();
				} else SynErr(151);
				break;
			}
			case 69: {
				Get();
				break;
			}
			case 70: {
				Get();
				break;
			}
			}
		}
		Expect(71);
	}

	void SkyParms() {
		if (la.kind == 72) {
			Get();
		} else if (la.kind == 73) {
			Get();
		} else SynErr(152);
		if (la.kind == 1) {
			Path();
		} else if (la.kind == 74) {
			Get();
		} else SynErr(153);
		Number();
		if (la.kind == 1) {
			Path();
		} else if (la.kind == 74) {
			Get();
		} else SynErr(154);
	}

	void DeformVertexes() {
		Expect(75);
		switch (la.kind) {
		case 31: {
			Get();
			Number();
			GeneratorFunction();
			Number();
			Number();
			Number();
			Number();
			break;
		}
		case 76: {
			Get();
			Number();
			if (la.kind == 2) {
				Number();
			} else if (StartOf(3)) {
				GeneratorFunction();
				Number();
				Number();
				Number();
			} else SynErr(155);
			break;
		}
		case 77: {
			Get();
			Number();
			Number();
			Number();
			GeneratorFunction();
			Number();
			Number();
			Number();
			Number();
			break;
		}
		case 78: {
			Get();
			Number();
			Number();
			Number();
			break;
		}
		case 79: {
			Get();
			break;
		}
		case 80: {
			Get();
			break;
		}
		case 81: {
			Get();
			break;
		}
		default: SynErr(156); break;
		}
	}

	void FogParms() {
		Expect(82);
		Expect(33);
		Number();
		Number();
		Number();
		Expect(34);
		Number();
	}

	void SurfaceParm() {
		Expect(83);
		switch (la.kind) {
		case 84: {
			Get();
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
		case 88: {
			Get();
			break;
		}
		case 89: {
			Get();
			break;
		}
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
		case 70: {
			Get();
			break;
		}
		default: SynErr(157); break;
		}
	}

	void ShaderDecl() {
		Path();
		Expect(54);
		while (StartOf(4)) {
			switch (la.kind) {
			case 72: case 73: {
				SkyParms();
				break;
			}
			case 54: {
				Stage();
				break;
			}
			case 75: {
				DeformVertexes();
				break;
			}
			case 82: {
				FogParms();
				break;
			}
			case 115: {
				Get();
				if (la.kind == 116) {
					Get();
				} else if (la.kind == 117) {
					Get();
				} else if (la.kind == 68) {
					Get();
				} else if (la.kind == 118) {
					Get();
				} else SynErr(158);
				break;
			}
			case 119: {
				Get();
				switch (la.kind) {
				case 2: {
					Number();
					break;
				}
				case 36: {
					Get();
					break;
				}
				case 111: {
					Get();
					break;
				}
				case 120: {
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
				default: SynErr(159); break;
				}
				break;
			}
			case 125: {
				Get();
				break;
			}
			case 126: {
				Get();
				break;
			}
			case 127: {
				Get();
				break;
			}
			case 36: {
				Get();
				break;
			}
			case 128: {
				Get();
				break;
			}
			case 129: {
				Get();
				Number();
				break;
			}
			case 130: {
				Get();
				Path();
				break;
			}
			case 131: {
				Get();
				break;
			}
			case 132: {
				Get();
				Number();
				Number();
				Number();
				Number();
				Number();
				Number();
				break;
			}
			case 133: {
				Get();
				Number();
				break;
			}
			case 134: {
				Get();
				Number();
				break;
			}
			case 135: {
				Get();
				Path();
				break;
			}
			case 136: {
				Get();
				Number();
				break;
			}
			case 137: {
				Get();
				Number();
				Number();
				break;
			}
			case 83: {
				SurfaceParm();
				break;
			}
			case 138: {
				Get();
				Path();
				break;
			}
			case 139: {
				Get();
				break;
			}
			case 140: {
				Get();
				Number();
				break;
			}
			}
		}
		Expect(71);
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
		{T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,T, x,T,x,x, x,x,T,x, x,x,x,x, x,x,T,x, x,x,x,T, x,x,x,T, T,T,x,x, x,T,x,x, x,T,T,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x},
		{x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x},
		{x,x,x,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, T,T,x,T, x,x,x,x, x,x,T,T, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,T, x,x,x,T, x,x,x,x, x,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,x,x}

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
			case 10: s = "\"GL_ZERO\" expected"; break;
			case 11: s = "\"GL_ONE\" expected"; break;
			case 12: s = "\"GL_SRC_COLOR\" expected"; break;
			case 13: s = "\"GL_ONE_MINUS_SRC_COLOR\" expected"; break;
			case 14: s = "\"GL_DST_COLOR\" expected"; break;
			case 15: s = "\"GL_ONE_MINUS_DST_COLOR\" expected"; break;
			case 16: s = "\"GL_SRC_ALPHA\" expected"; break;
			case 17: s = "\"GL_ONE_MINUS_SRC_ALPHA\" expected"; break;
			case 18: s = "\"GL_DST_ALPHA\" expected"; break;
			case 19: s = "\"GL_ONE_MINUS_DST_ALPHA\" expected"; break;
			case 20: s = "\"rgbGen\" expected"; break;
			case 21: s = "\"identityLighting\" expected"; break;
			case 22: s = "\"identity\" expected"; break;
			case 23: s = "\"entity\" expected"; break;
			case 24: s = "\"oneMinusEntity\" expected"; break;
			case 25: s = "\"vertex\" expected"; break;
			case 26: s = "\"oneMinusVertex\" expected"; break;
			case 27: s = "\"exactVertex\" expected"; break;
			case 28: s = "\"lightingDiffuse\" expected"; break;
			case 29: s = "\"LightingDiffuse\" expected"; break;
			case 30: s = "\"lightingSpecular\" expected"; break;
			case 31: s = "\"wave\" expected"; break;
			case 32: s = "\"const\" expected"; break;
			case 33: s = "\"(\" expected"; break;
			case 34: s = "\")\" expected"; break;
			case 35: s = "\"alphaGen\" expected"; break;
			case 36: s = "\"portal\" expected"; break;
			case 37: s = "\"tcGen\" expected"; break;
			case 38: s = "\"base\" expected"; break;
			case 39: s = "\"lightmap\" expected"; break;
			case 40: s = "\"environment\" expected"; break;
			case 41: s = "\"vector\" expected"; break;
			case 42: s = "\"tcMod\" expected"; break;
			case 43: s = "\"rotate\" expected"; break;
			case 44: s = "\"scale\" expected"; break;
			case 45: s = "\"scroll\" expected"; break;
			case 46: s = "\"stretch\" expected"; break;
			case 47: s = "\"transform\" expected"; break;
			case 48: s = "\"turb\" expected"; break;
			case 49: s = "\"entityTranslate\" expected"; break;
			case 50: s = "\"alphaFunc\" expected"; break;
			case 51: s = "\"GT0\" expected"; break;
			case 52: s = "\"LT128\" expected"; break;
			case 53: s = "\"GE128\" expected"; break;
			case 54: s = "\"{\" expected"; break;
			case 55: s = "\"map\" expected"; break;
			case 56: s = "\"$lightmap\" expected"; break;
			case 57: s = "\"$whiteimage\" expected"; break;
			case 58: s = "\"*white\" expected"; break;
			case 59: s = "\"clampmap\" expected"; break;
			case 60: s = "\"animMap\" expected"; break;
			case 61: s = "\"blendFunc\" expected"; break;
			case 62: s = "\"add\" expected"; break;
			case 63: s = "\"filter\" expected"; break;
			case 64: s = "\"blend\" expected"; break;
			case 65: s = "\"depthFunc\" expected"; break;
			case 66: s = "\"equal\" expected"; break;
			case 67: s = "\"lequal\" expected"; break;
			case 68: s = "\"disable\" expected"; break;
			case 69: s = "\"depthWrite\" expected"; break;
			case 70: s = "\"detail\" expected"; break;
			case 71: s = "\"}\" expected"; break;
			case 72: s = "\"skyParms\" expected"; break;
			case 73: s = "\"skyparms\" expected"; break;
			case 74: s = "\"-\" expected"; break;
			case 75: s = "\"deformVertexes\" expected"; break;
			case 76: s = "\"normal\" expected"; break;
			case 77: s = "\"move\" expected"; break;
			case 78: s = "\"bulge\" expected"; break;
			case 79: s = "\"autoSprite\" expected"; break;
			case 80: s = "\"autoSprite2\" expected"; break;
			case 81: s = "\"projectionShadow\" expected"; break;
			case 82: s = "\"fogparms\" expected"; break;
			case 83: s = "\"surfaceparm\" expected"; break;
			case 84: s = "\"alphashadow\" expected"; break;
			case 85: s = "\"areaportal\" expected"; break;
			case 86: s = "\"clusterportal\" expected"; break;
			case 87: s = "\"donotenter\" expected"; break;
			case 88: s = "\"flesh\" expected"; break;
			case 89: s = "\"fog\" expected"; break;
			case 90: s = "\"lava\" expected"; break;
			case 91: s = "\"metalsteps\" expected"; break;
			case 92: s = "\"nodamage\" expected"; break;
			case 93: s = "\"nodlight\" expected"; break;
			case 94: s = "\"nodraw\" expected"; break;
			case 95: s = "\"nodrop\" expected"; break;
			case 96: s = "\"noimpact\" expected"; break;
			case 97: s = "\"nomarks\" expected"; break;
			case 98: s = "\"nolightmap\" expected"; break;
			case 99: s = "\"nosteps\" expected"; break;
			case 100: s = "\"nonsolid\" expected"; break;
			case 101: s = "\"origin\" expected"; break;
			case 102: s = "\"playerclip\" expected"; break;
			case 103: s = "\"slick\" expected"; break;
			case 104: s = "\"slime\" expected"; break;
			case 105: s = "\"structural\" expected"; break;
			case 106: s = "\"trans\" expected"; break;
			case 107: s = "\"water\" expected"; break;
			case 108: s = "\"pointlight\" expected"; break;
			case 109: s = "\"forcefield\" expected"; break;
			case 110: s = "\"shotclip\" expected"; break;
			case 111: s = "\"sky\" expected"; break;
			case 112: s = "\"monsterclip\" expected"; break;
			case 113: s = "\"hint\" expected"; break;
			case 114: s = "\"ladder\" expected"; break;
			case 115: s = "\"cull\" expected"; break;
			case 116: s = "\"front\" expected"; break;
			case 117: s = "\"back\" expected"; break;
			case 118: s = "\"none\" expected"; break;
			case 119: s = "\"sort\" expected"; break;
			case 120: s = "\"opaque\" expected"; break;
			case 121: s = "\"banner\" expected"; break;
			case 122: s = "\"underwater\" expected"; break;
			case 123: s = "\"additive\" expected"; break;
			case 124: s = "\"nearest\" expected"; break;
			case 125: s = "\"nopicmip\" expected"; break;
			case 126: s = "\"nomipmaps\" expected"; break;
			case 127: s = "\"polygonOffset\" expected"; break;
			case 128: s = "\"entityMergable\" expected"; break;
			case 129: s = "\"tessSize\" expected"; break;
			case 130: s = "\"q3map_backshader\" expected"; break;
			case 131: s = "\"q3map_globaltexture\" expected"; break;
			case 132: s = "\"q3map_sun\" expected"; break;
			case 133: s = "\"light\" expected"; break;
			case 134: s = "\"q3map_surfacelight\" expected"; break;
			case 135: s = "\"q3map_lightimage\" expected"; break;
			case 136: s = "\"q3map_lightsubdivide\" expected"; break;
			case 137: s = "\"q3map_backsplash\" expected"; break;
			case 138: s = "\"qer_editorimage\" expected"; break;
			case 139: s = "\"qer_nocarve\" expected"; break;
			case 140: s = "\"qer_trans\" expected"; break;
			case 141: s = "??? expected"; break;
			case 142: s = "invalid GeneratorFunction"; break;
			case 143: s = "invalid BlendMode"; break;
			case 144: s = "invalid RgbGen"; break;
			case 145: s = "invalid AlphaGen"; break;
			case 146: s = "invalid TcGen"; break;
			case 147: s = "invalid TcMod"; break;
			case 148: s = "invalid AlphaFunc"; break;
			case 149: s = "invalid Stage"; break;
			case 150: s = "invalid Stage"; break;
			case 151: s = "invalid Stage"; break;
			case 152: s = "invalid SkyParms"; break;
			case 153: s = "invalid SkyParms"; break;
			case 154: s = "invalid SkyParms"; break;
			case 155: s = "invalid DeformVertexes"; break;
			case 156: s = "invalid DeformVertexes"; break;
			case 157: s = "invalid SurfaceParm"; break;
			case 158: s = "invalid ShaderDecl"; break;
			case 159: s = "invalid ShaderDecl"; break;

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