
#include <iostream>

static char puzzle[9*9+1] =
  "xxxx1xxxx"
  "2x63x8xx9"
  "5x8xxxx3x"
  "xxxxxxx2x"
  "6xx2xx5x4"
  "3xxxx9x18"
  "xxx1xx4xx"
  "xx27xxx85"
  "9x36xxxxx"
;

static const char square[9] = {
  0*9+0, 0*9+3, 0*9+6,
  3*9+0, 3*9+3, 3*9+6,
  6*9+0, 6*9+3, 6*9+6,
};

static const char subsquare[9] = {
  0*9+0, 0*9+1, 0*9+2,
  1*9+0, 1*9+1, 1*9+2,
  2*9+0, 2*9+1, 2*9+2,
};

int popcount(int x) {
  return __builtin_popcount(x);
  /*int res = 0;
  for (int i = 0; i != 9; ++i) {
    res += (x >> i) & 1;
  }
  return res;*/
}

void add_sideways(int &x1, int &x2, int &x4, int &x8, int &y1) {
  int cy2 = x1 & y1;
  x1 ^= y1;
  int cy4 = x2 & cy2;
  x2 ^= cy2;
  int cy8 = x4 & cy4;
  x4 ^= cy4;
  x8 ^= cy8;
}

int which_bit_set(int set) {
  if (set == 0 || (set & set-1)) {
    return -1; // zero or more bits set.
  }
  return 31 - __builtin_clz(set);
}

template <class AddrFn>
bool solve_unique(char *label, char *puzzle, int *sets, int is8, AddrFn f) {
  bool solved = false;
  for (int j = 0; j != 9; ++j) {
    char val = puzzle[f(j)];
    if (val == 'x' && (~sets[f(j)] & is8)) {
      int wbs = which_bit_set(~sets[f(j)] & is8);
      if (wbs == -1) { std::cout << "logic error"; return false; }
      puzzle[f(j)] = '1' + wbs;
      std::cout << "solved unique " << label << " " << (char)('1' + wbs) << std::endl;
      solved = true;
    }
  }
  return solved;
}

struct set {
  int bits;
};

std::string strset(int bits) {
  char chr[9];
  for (int i = 0; i != 9; ++i) {
    chr[i] = (char)(((bits >> i) & 1) ? '1' + i : '.');
  }
  return std::string(chr, chr+9);
}

bool calc(char *puzzle) {
  using namespace std;
  int sets[9*9] = {0};
  //char row_counts[9*9] = {0};
  //char col_counts[9*9] = {0};
  //char square_counts[9*9] = {0};

  for (int row = 0; row != 9; ++row) {
    for (int col = 0; col != 9; ++col) {
      char val = puzzle[row*9+col];
      if (val >= '1' && val <= '9') {
        int bits = 1 << (val-'1');
        sets[row*9+col] |= bits ^ 0x1ff;
        for (int i = 0; i != 9; ++i) {
          sets[i*9+col] |= bits;
          sets[row*9+i] |= bits;
        }
      }
    }
  }

  for (int s = 0; s != 9; ++s) {
    for (int ss = 0; ss != 9; ++ss) {
      char val = puzzle[square[s] + subsquare[ss]];
      if (val >= '1' && val <= '9') {
        int bits = 1 << (val-'1');
        for (int i = 0; i != 9; ++i) {
          sets[square[s] + subsquare[i]] |= bits;
        }
      }
    }
  }

  // Dump sets
  for (int row = 0; row != 9; ++row) {
    for (int col = 0; col != 9; ++col) {
      char val = puzzle[row*9+col];
      int set = sets[row*9+col];
      cout << " " << val << ((val == 'x' && popcount(set) == 8) ? " ->" : "   ") << strset(set);
    }
    cout << endl;
  }
  cout << endl;

  bool solved = false;

  // Solve "Sole Candidate".
  for (int row = 0; row != 9; ++row) {
    for (int col = 0; col != 9; ++col) {
      char val = puzzle[row*9+col];
      int set = sets[row*9+col];
      if (val == 'x' && popcount(set) == 8) {
        for (int i = 0; i != 9; ++i) {
          if (!((set >> i) & 1)) {
            puzzle[row*9+col] = '1' + i;
            cout << "solved sole " << (char)('1' + i) << endl;
            solved = true;
          }
        }
      }
    }
  }

  // Solve "Unique Candidate"
  for (int i = 0; i != 9; ++i) {
    int r1 = 0, r2 = 0, r4 = 0, r8 = 0;
    int c1 = 0, c2 = 0, c4 = 0, c8 = 0;
    int s1 = 0, s2 = 0, s4 = 0, s8 = 0;
    for (int j = 0; j != 9; ++j) {
      add_sideways(r1, r2, r4, r8, sets[i*9+j]);
      add_sideways(c1, c2, c4, c8, sets[j*9+i]);
      add_sideways(s1, s2, s4, s8, sets[square[i] + subsquare[j]]);
    }
    int row_is8 = r8 & ~r4 & ~r2 & ~r1;
    int col_is8 = c8 & ~c4 & ~c2 & ~c1;
    int square_is8 = s8 & ~s4 & ~s2 & ~s1;
    //cout << " row_is8=" << strset(row_is8) << endl;
    //cout << " col_is8=" << strset(col_is8) << endl;
    //cout << " square_is8=" << strset(square_is8) << endl;
    if (row_is8) {
      solved = solved || solve_unique("row", puzzle, sets, row_is8, [i](int j) { return i*9+j; });
    }
    if (col_is8) {
      solved = solved || solve_unique("col", puzzle, sets, col_is8, [i](int j) { return j*9+i; });
    }
    if (square_is8) {
      solved = solved || solve_unique("square", puzzle, sets, row_is8, [i](int j) { return square[i] + subsquare[j]; });
    }
  }
  return solved;
}


int main() {
  while (calc(puzzle)) ;
}

