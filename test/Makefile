sources := foo.c bar.c baz.s ugh.h
reverse = $(2) $(1)

$(info (subst ee,EE,feet on the street) ->  $(subst ee,EE,feet on the street))       #$(subst FROM,TO,TEXT)
$(info (patsubst %.c,%.o,x.c.c bar.c)   ->  $(patsubst %.c,%.o,x.c.c bar.c))         #$(patsubst PATTERN,REPLACEMENT,TEXT)   
$(info (sources:.c=.o)                  ->  $(sources:.c=.o))                        #$(VAR:SUFFIX=REPLACEMENT) == $(patsubst %SUFFIX,%REPLACEMENT,$(VAR))
$(info findstring包含关系,filter 全词匹配)
$(info (findstring a,a b c)             ->  $(findstring a,a b c))                   #$(findstring FIND,IN)
$(info (filter %.c %.s,$(sources))      ->  $(filter %.c %.s,$(sources)))            #$(filter PATTERN...,TEXT)
$(info (filter-out foo.c,$(sources))    ->  $(filter-out foo.c,$(sources)))          #$(filter-out PATTERN...,TEXT)
$(info (sort foo bar lose foo)          ->  $(sort foo bar lose foo))                #$(sort LIST)
$(info (word 2, foo bar baz)            ->  $(word 2, foo bar baz))                  #$(word N,TEXT)
$(info (wordlist 2, 3, foo bar baz)     ->  $(wordlist 2, 3, foo bar baz))           #$(wordlist S,E,TEXT)
$(info (words foo bar)                  ->  $(words foo bar))                        #$(words TEXT)
$(info (firstword foo bar)              ->  $(firstword foo bar))                    #$(firstword NAMES...)

$(info (dir src/foo.c hacks)            ->  $(dir src/foo.c hacks))                  #$(dir NAMES...)
$(info (notdir src/foo.c hacks)         ->  $(notdir src/foo.c hacks))               #$(notdir NAMES...)
$(info (suffix src/foo.c src-1.0/bar.c hacks)   ->  $(suffix src/foo.c src-1.0/bar.c hacks)) #$(suffix NAMES...)
$(info (basename src/foo.c src-1.0/bar.c /home/jack/.font.cache-1 hacks)    ->  $(basename src/foo.c src-1.0/bar.c /home/jack/.font.cache-1 hacks))
$(info (addsuffix .c,foo bar)           ->  $(addsuffix .c,foo bar))
$(info (addprefix src/,foo bar)         ->  $(addprefix src/,foo bar))
$(info (join a b c , .c .o)             ->  $(join a b c , .c .o))
$(info (wildcard *.c)                   ->  $(wildcard *.c))
$(info (foreach dir,$(dirs),$(wildcard $(dir)/*))   ->  $(foreach dir,$(dirs),$(wildcard $(dir)/*)))
$(info (if $(SRC_DIR) $(SRC_DIR),/home/src)         ->  $(if $(SRC_DIR) $(SRC_DIR),/home/src))   #如果“SRC_DIR”变量值不为空，则将变量“SRC_DIR”指定 的目录作为一个子目录
$(warning (call reverse,a,b)    ->  $(call reverse,a,b))

hellomake: test.cpp
	gcc -o hellomake test.cpp -I.

clean:
	rm hellomake
