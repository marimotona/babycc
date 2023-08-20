#### tokenize.c
入力された文字列をトークンに分割し種類ごとに分ける

##### equal関数
・Tokenオブジェクトのポインタtok
・文字列op

tokのlocと、opのメモリ領域の文字列が同じかどうかを判断している

トークンtokが、指定された文字列opと完全に一致しているかどうか

tok->locは、トークンの開始位置を表す

ex:
```c
Token token;
token.loc = "==";
token.len = 2;

bool result1 = equal(&token, "=="); // true
bool result2 = equal(&token, "=");  // false
bool result3 = equal(&token, "==="); // false
```

##### skip関数
現在のトークンが一致する場合は次のトークンを返す
一致しない場合はエラーを返す

この関数で、文法を定義している
ソースコードが期待される文法に従って記述されているかを確認している

だから、static関数ではない
parse.cの中で呼び出されている


##### new_token関数
呼び出されるごとに、kind start endを使用し新しいTokenを作成する

1. calloc関数で、Token構造体のサイズに合わせてメモリ領域を確保
2. 確保されたメモリ領域のアドレスをtokというポインタ変数に代入
3. tok->kindに、kindを代入し、トークンの種類をセット
4. tok->locに、startを入れ、文字列の開始位置をセット
5. tok->lenに、end - startでトークンの長さをセット


##### tokenize関数
+*/-(){}などの解析はここで行わず、トークンの長さを取得しそのトークンを
トークンリストに追加している

トークンがどのような演算子であるかどうかは、ここでは
解析されていない



#### parse.c
分割されたトークンごとに、解析を行い、
それを抽象構文木（AST）に変換を行う

-----------------------------------------------------

・new_node, new_binary, new_unary, new_num：
ASTノードを生成するためのヘルパ関数

・expr：
式を解析するためのエントリーポイント

・equality, relational, add, mul, unary, primary：
特定の演算子や式の部分を解析するための関数。これらは演算子の優先順序を考慮して階層的に定義されている

・parse：
トークンストリームを受け取り、ASTを生成するための関数。この関数はexprを呼び出してASTのルートノードを生成する

-----------------------------------------------------


### step9
expr -> stmt に変化
式を最初に解析し始めるのではなく、文から初めに解析を行う流れに変更

#### parse.c

##### assign関数

equality関数で、左辺の式を解析しnode変数に代入している

もし、次のトークンが（'='）代入演算子だった場合、
再帰的に、assign(&tok, tok->next)を呼び出し、右辺の式を解析する

new_binary関数で、node（左辺）、assign（右辺）を持つ
ＡＳＴノードを作成する

*rest = tok;で、解析が終わったトークンの位置を返す

**rest:**
パーサ関数が解析を終了した時点のトークンの位置を返す
再帰的に呼び出した際に、次の解析場所がどこか分かるようになる

関数が呼び出されると、restには現在のトークンの位置を指す
ポインタが渡される

解析が終わった後に、restに解析が終了した時点の一を渡し、更新をする

よって、次の位置から解析を行うことが出来る


#### stmt

ND_EXPR_STMTノードでラップをしている
ND_EXPR_STMTノードは式文を表すようにしている

1. parse関数はトークン列を引数に受け取り、ASTを構築する
   これが、stmt関数を通じて行われる。
2. stmt関数は、トークンをexpr_stmt関数に渡し、式を解析
   その結果をND_EXPR_STMTノードにラップして返す



### step10

#### parse.c

##### new_var_node関数

変更前の実装
```c
static Node *new_var_node(char name) {
    Node *node = new_node(ND_VAR);
    node->name = name;
    return node;
}
```
単一の文字(char型)を受けとって、それをnode->nameフィールドに設定

変更後の実装
```c
static Node *new_var_node(Obj *var) {
    Node *node = new_node(ND_VAR);
    node->var = var;
    return node;
}
```
Obj型のポインタを使用することで、複数の文字からなる変数をサポートしている


##### primary関数

```c
if (tok->kind == TK_IDENT) {
    Obj *var = find_var(tok);
    if(!var)
        var = new_lvar(strndup(tok->loc, tok->len)); 
    *rest = tok->next;
    return new_var_node(var);
}
```
strndup(*tok->loc)
トークンの位置情報から変数名をコピー
new_lvar関数に渡す

```c
strndup(tok->loc, tok->len)
```
指定された文字列から指定された文字列の長さの部分文字列をコピーし、
新しいメモリ領域に格納し、そのポインタを返す

トークンが表している変数名が新しいメモリ領域に格納されている


最後にreturn new_var_node(var);
で、ASTノードを作成しノードを返す

#### tokenize.c

##### tokenize関数

```c
if(is_ident1(*p)) {
    char *start = p;
    do {
        p++;
    } while (is_ident2(*p));
    cur = cur->next = new_token(TK_IDENT, start, p);
    continue;
}
```

is_ident1で、最初の文字が有効な文字か確認

```c
static bool is_ident1(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}
```

有効な場合は、開始位置を保持しておく
2文字目以降も、条件に合っているか確認

```c
cur = cur->next = new_token(TK_IDENT, start, p);
```
終点を見つけたら、startから、p（終わり）までの文字列をトークナイズする


##### Function

Node型はASTの一部を表すのに対して、Function型は完全な関数を表現する
Functuionという一つの大きなもので、今まで作成した情報をラップしている

parse.cの中で、Function型のオブジェクトを作成している

```c
Function *parse(Token *tok) {
    Node head;
    Node *cur = &head;
    while (tok->kind != TK_EOF)
      cur = cur->next = stmt(&tok, tok);
    Function *prog = calloc(1, sizeof(Function));
    prog->body = head.next;
    prog->locals = locals;
    return prog;
}
```
parse関数の最後の部分で、
Function型のオブジェクトとして、
・関数の本体
・ローカル変数、スタックのサイズ
を保持したオブジェクトを作成している


## step11

#### tokenize.c

##### convert_keywords関数
```c
static void convert_keywords(Token *tok) {
    for (Token *t= tok; t->kind != TK_EOF; t = t->next)
        if(equal(t, "return"))
            t->kind = TK_KEYWORD;
}
```
引数には、連結リストが渡される
終わりまで見ていき、'return'が会った多彩には、トークンの種類を'TK_KEYWORD'にする


#### parse.c

##### stmt関数

```c
static Node *stmt(Token **rest, Token *tok) {
    if (equal(tok, "return")) {
      Node *node = new_unary(ND_RETURN, expr(&tok, tok->next));
      *rest = skip(tok, ";");
      return node;
    }
    return expr_stmt(rest, tok);
}
```
文のNodeオブジェクトを構築する

もし、return文だった場合には、new_unary関数を呼び出し、
ND_RETURN型のNodeオブジェクトを作成する