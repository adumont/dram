#define __ASSERT_USE_STDERR
#include <assert.h>

// We map PINs
// Control Signals, Active Low
#define DIMM_CAS_ 2
#define DIMM_W_   11
#define DIMM_RAS_ 12

// Address signals, we only play with 4 for now
#define DIMM_A0 4
#define DIMM_A1 5
#define DIMM_A2 7
#define DIMM_A3 8

// Data signals, we only play with 4 for now
#define DIMM_DQ0 3
#define DIMM_DQ1 6
#define DIMM_DQ2 9
#define DIMM_DQ3 10

// extracts bit N of integer I:
#define BIT(I,N) (((I)>>(N)) & 1 )

// N-least significant bits of I
#define LSB(I,N) ( (I)        & ((1<<N) - 1) )

// N-most significant bits of I
#define MSB(I,N) ( ((I)>>(N)) & ((1<<N) - 1) )

// recompose 4 bits to integer
#define COMPOSE(B3,B2,B1,B0) ( (B3)<<3 + (B2)<<2 + (B1)<<1 + (B0)<<0 )

void setup() {
  initDRAM();

  // put your setup code here, to run once:
  Serial.begin(115200); //start serial communication at 115200 bps
  Serial.println("Let's go");

//  Serial.println("Test 1");
//  for( int a=0; a<256; a++) {
//    Serial.print("ADDR=");
//    Serial.println(a);
//    for( int d=0; d<16; d++) {
//      Serial.print("  ");
//      Serial.print(d);
//      writeDRAM(a,d);
//      Serial.print(" -> ");
//      int r=readDRAM(a);
//      Serial.print(r);
//      Serial.print("\t");
//      if(d==r) {
//        Serial.println("OK");
//      } else {
//        Serial.println("FAIL");
//      }
//    }
//  }

  Serial.println("Test 2");
  for( int a=0; a<256; a++) {
    for( int d=0; d<16; d++) {
      writeDRAM(a,d);
      int r=readDRAM(a);
      if(d!=r) {
        Serial.print("ADDR=");
        Serial.println(a);
          Serial.print("  ");
          Serial.print(d);
          Serial.print(" -> ");
          Serial.print(r);
          Serial.print("\t");
        Serial.println("FAIL");
      }
    }
  }


  Serial.println("Test 3");
  for( int d=0; d<16; d++) {
//    Serial.print("DATA=");
//    Serial.println(d);
    for( int a=0; a<256; a++) {
      writeDRAM(a,d);
    }
    for( int a=0; a<16; a++) {
      int r=readDRAM(a);
      if(d!=r) {
          Serial.print("  ");
          Serial.print(a);
          Serial.print(" -> ");
          Serial.print(r);
          Serial.print("\t");
        Serial.println("FAIL");
      }
    }
  }

  Serial.println("Test 4");
  assert( (int)MSB(16,4) == 1 );
  assert( (int)LSB(16,4) == 0 );

  writeDRAM( 0,13);
  writeDRAM(16, 9);
  
  assert( readDRAM(0) == 13 );
  assert( readDRAM(16) == 9 );

  writeDRAM( 0,15);
  writeDRAM(16,15);
  
  assert( readDRAM(0) == 15 );
  assert( readDRAM(16) == 15 );

  assert( dumpDRAM() == 3840 ); // F00

  Serial.println("End of tests");

}

String s;
char cbuff[40];

char c;
int a, d;

void loop()
{
   c="";
   a=0;
   d=0;
  //Serial.println("Enter instruction");
  if ((Serial.available()>0)) {
    s = Serial.readStringUntil('\n');
    s.toCharArray(cbuff,s.length()+1);
    sscanf(cbuff, "%c %x %x", &c, &a, &d);
    if(c==119) { // w
      writeDRAM(a,d);
    }
    if(c==114) { // r 
      Serial.print("[");
      Serial.print(a,HEX);
      Serial.print("]=");
      Serial.println((int)readDRAM(a),HEX);
    }
    if(c==115) { // s
      dumpDRAM();
    }
  }
}

int dumpDRAM() {
  int sum, t;
  sum= 0;
  Serial.println("");
  Serial.println("Memory dump:");
  Serial.println("");
  Serial.print("   ");
  for(char i=0; i<16; i++) {
    Serial.print(i,HEX);
    Serial.print(" ");
  }
  Serial.println("");
  for(char i=0; i<16; i++) {
    Serial.print(i,HEX);
    Serial.print(": ");
    for(char j=0; j<16; j++) {
      t=(int)readDRAM(16*i+j);
      sum+=t;
      Serial.print(t,HEX);
      Serial.print(" ");
    }
    Serial.println("");
  }
  Serial.println("");
  Serial.print("sum=");
  Serial.print(sum,HEX);
  Serial.println("");
  return sum;
}

void initDRAM() {
  pinMode(DIMM_CAS_ , OUTPUT);
  pinMode(DIMM_RAS_ , OUTPUT);
  pinMode(DIMM_W_   , OUTPUT);

  pinMode(DIMM_A0 , OUTPUT);
  pinMode(DIMM_A1 , OUTPUT);
  pinMode(DIMM_A2 , OUTPUT);
  pinMode(DIMM_A3 , OUTPUT);

  // Default state, CAS_, RAS_ & W_ are High
  digitalWrite(DIMM_CAS_, HIGH);
  digitalWrite(DIMM_RAS_, HIGH);
  digitalWrite(DIMM_W_  , HIGH);

  delay(200); // wait at least 100 microseconds
}

void writeAddr(int A) {
  // Write ADDR
  digitalWrite(DIMM_A0, BIT(A,0));
  digitalWrite(DIMM_A1, BIT(A,1));
  digitalWrite(DIMM_A2, BIT(A,2));
  digitalWrite(DIMM_A3, BIT(A,3));
}

void writeDRAM(int A, char D) {
  
  // write Row Addr
  writeAddr( MSB(A,4) );
  
  // Write DATA
  pinMode(DIMM_DQ0, OUTPUT);
  pinMode(DIMM_DQ1, OUTPUT);
  pinMode(DIMM_DQ2, OUTPUT);
  pinMode(DIMM_DQ3, OUTPUT);

  digitalWrite(DIMM_DQ0, BIT(D,0)); // 1
  digitalWrite(DIMM_DQ1, BIT(D,1)); // 2
  digitalWrite(DIMM_DQ2, BIT(D,2)); // 4
  digitalWrite(DIMM_DQ3, BIT(D,3)); // 8
  delay(0);

  // DRAM WRITE
//  Serial.println("DRAM WRITE");
//  Serial.println("  RAS to low");
  digitalWrite(DIMM_RAS_, LOW);
//  Serial.println("  Read/Write to Write state");

  // write Row Addr
  writeAddr( LSB(A,4) );

  digitalWrite(DIMM_W_  , LOW);

//  Serial.println("  CAS to low");
  digitalWrite(DIMM_CAS_, LOW);
//  Serial.println("  Read/Write back to Read state");
  digitalWrite(DIMM_W_  , HIGH);
//  Serial.println("  CAS to high");
  digitalWrite(DIMM_CAS_, HIGH);
//  Serial.println("  RAS to high");
  digitalWrite(DIMM_RAS_, HIGH);

  delay(0);
}

char readDRAM(int A) {
  int rd = 0; // Data we'll read

  pinMode(DIMM_DQ0, INPUT);
  pinMode(DIMM_DQ1, INPUT);
  pinMode(DIMM_DQ2, INPUT);
  pinMode(DIMM_DQ3, INPUT);

  // write Row Addr
  writeAddr( MSB(A,4) );

  //  Serial.println("  RAS to low");
  digitalWrite(DIMM_RAS_, LOW);

  //  Serial.println("  Read/Write to Read state");
  digitalWrite(DIMM_W_, HIGH);

  // write Row Addr
  writeAddr( LSB(A,4) );

  //  Serial.println("  CAS to low");
  digitalWrite(DIMM_CAS_, LOW);

  //Serial.println("Save data to variable");

  rd += digitalRead(DIMM_DQ0) << 0;
  rd += digitalRead(DIMM_DQ1) << 1;
  rd += digitalRead(DIMM_DQ2) << 2;
  rd += digitalRead(DIMM_DQ3) << 3;
  
  //  Serial.println("CAS to high");
  digitalWrite(DIMM_CAS_, HIGH);

  //  Serial.println("RAS to high");
  digitalWrite(DIMM_RAS_, HIGH);

  //Serial.print("Read data: ");
  //Serial.println(rd);

  return rd;
 
}

// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.print(F("ASSERT FAILED IN: "));
    Serial.println(__file);
    Serial.print(__func);
    Serial.print(F(": "));
    Serial.print(__lineno, DEC);
    Serial.print(F(": "));
    Serial.println(__sexp);
    Serial.flush();
    // abort program execution.
    abort();
}
