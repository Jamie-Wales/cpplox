#pragma once

enum OP_CODE { CONSTANT, CONSTANT_LONG, RETURN };

struct constant {
  double value;
};

struct lineStart {
  int start;
  int lineNumber;
};

void printValue(double value);
