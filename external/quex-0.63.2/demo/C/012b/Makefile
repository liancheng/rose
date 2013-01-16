ENGINE = \
		boeck_Lexer.c   \
		max_Lexer.c     \
		moritz_Lexer.c             

ifdef ASSERTS_ENABLED_F
	CCFLAG_ASSERTS=# By default asserts are enabled
else 
	CCFLAG_ASSERTS=-DQUEX_OPTION_ASSERTS_DISABLED
endif

lexer: $(ENGINE:.c=.o) lexer.o Common-token.o
	gcc $^ -ggdb -o  $@ -I$(QUEX_PATH) `icu-config --ldflags` 
	 
%.o: %.c $(ENGINE_SOURCES)
	gcc -c $< -ggdb -Wall -o $@ \
		-I$(QUEX_PATH) \
		$(CCFLAG_ASSERTS) 

# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

COMMON_TOKEN_OPTIONS = \
	 --token-class-file Common-token.h \
     --token-policy single --no-warning-on-no-token-queue
		 # --no-message-on-extra-options \

%.E: %.c
	gcc -E $< -o  $@ -I$(QUEX_PATH)
	
boeck_Lexer.c:  boeck.qx Common-token.c Common-token_ids.h
	quex -i $< -o boeck::Lexer --icu -b 4 $(COMMON_TOKEN_OPTIONS) --language C 

max_Lexer.c:    max.qx Common-token.c Common-token_ids.h
	quex -i $< -o max::Lexer --icu -b 4 $(COMMON_TOKEN_OPTIONS) --language C 

moritz_Lexer.c: moritz.qx Common-token.c Common-token_ids.h
	quex -i $< -o moritz::Lexer --icu -b 4 $(COMMON_TOKEN_OPTIONS) --language C 

Common-token_ids.h \
Common-token.c: 
	quex -i token_ids.qx               \
		 --icu -b 4                    \
		 --token-id-prefix TKN_        \
		 -o Common                     \
		 --token-class Common::Token   \
		 --token-class-only            --language C

clean:
	rm -rf $(ENGINE)
	rm -rf $(ENGINE:.c=.o)
	rm -rf $(ENGINE:.c=.h)
	rm -rf $(ENGINE:.c=-token_ids.h)
	rm -rf $(ENGINE:.c=-token.h)
	rm -rf $(ENGINE:.c=-configuration.h)
	rm -rf Common-token*
	rm -rf lexer
	rm -rf *.E
	rm -rf lexer.o
