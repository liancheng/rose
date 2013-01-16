ENGINE = \
		boeck_Lexer.cpp   \
		max_Lexer.cpp     \
		moritz_Lexer.cpp             

ifdef ASSERTS_ENABLED_F
	CCFLAG_ASSERTS=# By default asserts are enabled
else 
	CCFLAG_ASSERTS=-DQUEX_OPTION_ASSERTS_DISABLED
endif

lexer: $(ENGINE:.cpp=.o) lexer.o Common-token.o
	g++ $^ -ggdb -o  $@ -I$(QUEX_PATH) `icu-config --ldflags`
	 
%.o: %.cpp $(ENGINE_SOURCES)
	g++ -c $< -ggdb -Wall -o $@ \
		-I$(QUEX_PATH) \
		$(CCFLAG_ASSERTS) 

# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

COMMON_TOKEN_OPTIONS = \
	 --token-class-file Common-token \
     --token-policy single --no-warning-on-no-token-queue
		 # --no-message-on-extra-options \

%.E: %.cpp
	g++ -E $< -o  $@ -I$(QUEX_PATH) 
	
boeck_Lexer.cpp:  boeck.qx Common-token.cpp Common-token_ids
	quex -i $< -o boeck::Lexer --icu -b 4 $(COMMON_TOKEN_OPTIONS) 

max_Lexer.cpp:    max.qx Common-token.cpp Common-token_ids
	quex -i $< -o max::Lexer --icu -b 4 $(COMMON_TOKEN_OPTIONS) 

moritz_Lexer.cpp: moritz.qx Common-token.cpp Common-token_ids
	quex -i $< -o moritz::Lexer --icu -b 4 $(COMMON_TOKEN_OPTIONS) 

Common-token_ids \
Common-token.cpp: 
	quex -i token_ids.qx               \
		 --icu -b 4                    \
		 --token-id-prefix TKN_        \
		 -o Common                     \
		 --token-class Common::Token   \
		 --token-class-only 

clean:
	rm -rf $(ENGINE)
	rm -rf $(ENGINE:.cpp=.o)
	rm -rf $(ENGINE:.cpp=)
	rm -rf $(ENGINE:.cpp=-token_ids)
	rm -rf $(ENGINE:.cpp=-token)
	rm -rf $(ENGINE:.cpp=-configuration)
	rm -rf Common-token*
	rm -rf lexer
	rm -rf *.E
	rm -rf lexer.o
