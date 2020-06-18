//===-------- KudeyarInstrFormats.td ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Year: 2017
// Alexey Shistko     alexey@kudeyar.com
// Andrei Safronov    andrei@kudeyar.com
//===----------------------------------------------------------------------===//
class Format<bits<5> val> 
{
  bits<5> Value = val;
}  
 
def Frm_Pseudo : Format<0>;
def Frm_0 : Format<1>;
def Frm_R : Format<2>;
def Frm_M : Format<3>;
def Frm_RR : Format<4>;
def Frm_RI4 : Format<5>;
def Frm_RI5 : Format<6>;
def Frm_RM : Format<7>;
def Frm_MR : Format<8>;
def Frm_RMA: Format<9>;
def Frm_MI4 : Format<10>;
def Frm_MI5 : Format<11>;
def Frm_RI32 : Format<12>;
def Frm_Bs : Format<13>;
def Frm_Bl : Format<14>;
def Frm_J : Format<15>;
def Frm_Rlist : Format<16>;

// Generic Kudeyar Format
class KudeyarInst<int size, dag outs, dag ins, string asmstr, 
  list<dag> pattern, Format f> : Instruction 
{
  Format Form = f;

  let Namespace = "Kudeyar";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let Size = size;
  let AsmString   = asmstr;
  let Pattern = pattern;

  bits<5> FormBits = Form.Value;
  let TSFlags{4-0} = FormBits;
}

class KudeyarInst16<dag outs, dag ins, string asmstr, 
  list<dag> pattern, Format f> : KudeyarInst<2, outs, ins, asmstr, pattern, f>
{
  field bits<16> Inst;

  let Inst{15-14} = 0x0;
}

class KudeyarInst32<dag outs, dag ins, string asmstr, list<dag> pattern,
  Format f> : KudeyarInst<4, outs, ins, asmstr, pattern, f>
{
  field bits<32> Inst;

  let Inst{31-30} = 0x1;
}

class KudeyarInst48<dag outs, dag ins, string asmstr, list<dag> pattern,
  Format f> : KudeyarInst<6, outs, ins, asmstr, pattern, f>
{
  field bits<48> Inst;

  let Inst{47-46} = 0x2;
}

class Inst_0<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst16<outs, ins, asmstr, pattern, Frm_0>
{
  bits<14> op;

  let Inst{13-0} = op;
}

class Inst_R<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst16<outs, ins, asmstr, pattern, Frm_R>
{
  bits<10> op;
  bits<4> Rd;

  let Inst{13-4} = op;
  let Inst{3-0} = Rd;  
}

class Inst_M<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst32<outs, ins, asmstr, pattern, Frm_M>
{
  bits<10> op;
  bits<4> Rb;
  bits<2> size;
  bits<2> amode;
  bits<11> disp;

  let amode = 0;
  let size = 0x2;

  let Inst{29-20} = op;
  let Inst{19-16} = Rb;
  let Inst{15} = 1;
  let Inst{14-13} = size;
  let Inst{12-11} = amode;
  let Inst{10-0} = disp;  
}

class Inst_RR<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst16<outs, ins, asmstr, pattern, Frm_RR>
{
  bits<6> op;
  bits<4> Rs;
  bits<4> Rd;

  let Inst{13-8} = op;
  let Inst{7-4} = Rs;
  let Inst{3-0} = Rd;  
}

class Inst_RI4<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst16<outs, ins, asmstr, pattern, Frm_RI4>
{
  bits<6> op;
  bits<4> Imm;
  bits<4> Rd;

  let Inst{13-8} = op;
  let Inst{7-4} = Imm;
  let Inst{3-0} = Rd;  
}

class Inst_RI5<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst16<outs, ins, asmstr, pattern, Frm_RI5>
{
  bits<5> op;
  bits<5> Imm;
  bits<4> Rd;

  let Inst{13-9} = op;
  let Inst{8-4} = Imm;
  let Inst{3-0} = Rd;  
}

class Inst_RM<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst32<outs, ins, asmstr, pattern, Frm_RM>
{
  bits<6> op;
  bits<4> Rb;
  bits<4> Rd;
  bits<2> size;
  bits<2> amode;
  bits<11> disp;

  let amode = 0;

  let Inst{29-24} = op;
  let Inst{23-20} = Rb;
  let Inst{19-16} = Rd;
  let Inst{15} = 0;
  let Inst{14-13} = size;
  let Inst{12-11} = amode;
  let Inst{10-0} = disp;  
}

class Inst_MR<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst32<outs, ins, asmstr, pattern, Frm_MR>
{
  bits<6> op;
  bits<4> Rb;
  bits<4> Rs;
  bits<2> size;
  bits<2> amode;
  bits<11> disp;

  let amode = 0;

  let Inst{29-24} = op;
  let Inst{23-20} = Rb;
  let Inst{19-16} = Rs;
  let Inst{15} = 1;
  let Inst{14-13} = size;
  let Inst{12-11} = amode;
  let Inst{10-0} = disp;  
}

class Inst_MI4<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst32<outs, ins, asmstr, pattern, Frm_MI4>
{
  bits<6> op;
  bits<4> Rb;
  bits<4> Imm;
  bits<2> size;
  bits<2> amode;
  bits<11> disp;

  let amode = 0;

  let Inst{29-24} = op;
  let Inst{23-20} = Imm;
  let Inst{19-16} = Rb;
  let Inst{15} = 1;
  let Inst{14-13} = size;
  let Inst{12-11} = amode;
  let Inst{10-0} = disp;  
}

class Inst_MI5<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst32<outs, ins, asmstr, pattern, Frm_MI5>
{
  bits<5> op;
  bits<4> Rb;
  bits<5> Imm;
  bits<2> size;
  bits<2> amode;
  bits<11> disp;

  let amode = 0;

  let Inst{29-25} = op;
  let Inst{24-20} = Imm;
  let Inst{19-16} = Rb;
  let Inst{15} = 1;
  let Inst{14-13} = size;
  let Inst{12-11} = amode;
  let Inst{10-0} = disp;  
}

class Inst_RI32<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst48<outs, ins, asmstr, pattern, Frm_RI32>
{
  bits<6> op;
  bits<4> Rs;
  bits<4> Rd;
  bits<32> Imm;

  let Inst{45-40} = op;
  let Inst{39-36} = Rs;
  let Inst{35-32} = Rd;
  let Inst{31-0} = Imm;  
}

class Inst_RMA<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst48<outs, ins, asmstr, pattern, Frm_RMA>
{
  bits<6> op;
  bits<1> sext;
  bits<1> dir;
  bits<2> size;
  bits<4> Rd;
  bits<32> address;

  let Inst{45-40} = op;
  let Inst{39} = sext;
  let Inst{38} = dir;
  let Inst{37-36} = size;
  let Inst{35-32} = Rd;
  let Inst{31-0} = address; 
}

class Inst_Bs<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst16<outs, ins, asmstr, pattern, Frm_Bs>
{
  bits<6> op;
  bits<8> offset;
  
  let Inst{13-8} = op;
  let Inst{7-0} = offset;  
}

class Inst_Bl<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst32<outs, ins, asmstr, pattern, Frm_Bl>
{
  bits<6> op;
  bits<24> offset;

  let Inst{29-24} = op;
  let Inst{23-0} = offset;  
}

class Inst_J<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst32<outs, ins, asmstr, pattern, Frm_J>
{
  bits<6> op;
  bits<24> address;

  let Inst{29-24} = op;
  let Inst{23-0} = address;  
}

class Inst_Rlist<dag outs, dag ins, string asmstr, list<dag> pattern> : 
  KudeyarInst32<outs, ins, asmstr, pattern, Frm_Rlist>
{
  bits<14> op;
  bits<16> reglist;

  let Inst{29-16} = op;
  let Inst{15-0} = reglist;  
}

// Pseudo instructions
class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
  : KudeyarInst<100, outs, ins, asmstr, pattern, Frm_Pseudo> 
{
  field bits<16> Inst;

  let Inst = 0x0;
}
