#! /bin/bash

gcc_opt='-Wall -O1 -D_FORTIFY_SOURCE=1'

# コマンドラインオプションの処理
for opt in $*
do
    case $opt in
	-d)
	    gcc_opt='-Wall -O1 -D_FORTIFY_SOURCE=1 -g -DDEBUG'
	    ;;
    esac
done

cd src
yacc -d parser.y
yr=$?
lex lexer.l
lr=$?
cd ..
if [ $yr = 0 -a $lr = 0 ] # both success
then
    if [ ! -d bin ]
    then
	mkdir bin
    fi
    gcc -o bin/lang $gcc_opt src/lang.c src/builtin.c src/main.c src/create.c src/eval.c src/y.tab.c src/lex.yy.c
fi
