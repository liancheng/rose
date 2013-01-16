ENGINE = \
		boeck_Lexer.c    \
		max_Lexer.c      \
		moritz_Lexer.c             

OBJS = $(ENGINE:.c=.o) lexer.o

ifdef ASSERTS_ENABLED_F
	CCFLAG_ASSERTS=# By default asserts are enabled
else 
	CCFLAG_ASSERTS=-DQUEX_OPTION_ASSERTS_DISABLED
endif

lexer: $(OBJS)
	gcc $(OBJS) -ggdb -o  $@ -I$(QUEX_PATH) 
# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

%.o: %.c 
	gcc -c $< -ggdb -Wall -o $@ -I$(QUEX_PATH) $(CCFLAG_ASSERTS) 
# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

%.E: %.c
	gcc -E $< -o  $@ -I$(QUEX_PATH) 
# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

max_Lexer.c: max.qx
	quex -i token.qx $< -o max::Lexer --iconv -b 4     \
		 --token-id-prefix MAX_TKN_ --token-policy single \
		 --language C                                  \
		 --no-warning-on-no-token-queue                

moritz_Lexer.c: moritz.qx
	quex -i token.qx $< -o moritz::Lexer --iconv -b 2     \
		 --token-id-prefix MORITZ_TKN_ --token-policy single \
		 --language C                                     \
		 --no-warning-on-no-token-queue                

boeck_Lexer.c: boeck.qx
	quex -i token.qx $< -o boeck::Lexer --codec utf8 \
		 --token-id-prefix TKN_ --token-policy single   \
		 --language C                                \
		 --no-warning-on-no-token-queue

clean:
	rm -rf boeck_Lexer*
	rm -rf max_Lexer*
	rm -rf moritz_Lexer*
	rm -rf lexer
	rm -rf *.E
	rm -rf lexer.o
