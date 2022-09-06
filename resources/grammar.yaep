TERM NUMBER;
E : T # 0
  | E '+' T # plus (0 2)
  ;

T : F # 0
  | T '*' F # mult (0 2)
  ;

F : NUMBER # 0
  | '(' E ')' # 1
  ;
