将 wrap.c 制作成动态库并移到lib库里
inc目录包含 wrap.c 和 wrap.h，其中 wrap.h 参与编译

gcc -c wrap.c -o wrap.o -fPIC
gcc -shared -o libwrap.so wrap.o
mv libwrap.so /usr/lib
