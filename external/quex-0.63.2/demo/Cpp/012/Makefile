ENGINE = \
		boeck_Lexer.cpp              \
		max_Lexer.cpp                \
		moritz_Lexer.cpp             

OBJS = $(ENGINE:.cpp=.o) lexer.o

ifdef ASSERTS_ENABLED_F
	CCFLAG_ASSERTS=# By default asserts are enabled
else 
	CCFLAG_ASSERTS=-DQUEX_OPTION_ASSERTS_DISABLED
endif

lexer: $(OBJS)
	g++ $(OBJS) -ggdb -o  $@ \
		-I$(QUEX_PATH) 
	 
# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

%.o: %.cpp $(ENGINE_SOURCES)
	g++ -c $< -ggdb -Wall -o $@ \
		-I$(QUEX_PATH) \
		$(CCFLAG_ASSERTS) 

# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

%.E: %.cpp
	g++ -E $< -o  $@ \
		-I$(QUEX_PATH) 
		
# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

max_Lexer.cpp:    max.qx
	quex -i token.qx $< -o max::Lexer --iconv -b 4     \
		 --token-id-prefix MAX_TKN_ --token-policy single \
		 --no-warning-on-no-token-queue

moritz_Lexer.cpp: moritz.qx
	quex -i token.qx $< -o moritz::Lexer --iconv -b 2     \
		 --token-id-prefix MORITZ_TKN_ --token-policy single \
		 --no-warning-on-no-token-queue

boeck_Lexer.cpp:  boeck.qx
	quex -i token.qx $< -o boeck::Lexer --codec utf8 \
		 --token-id-prefix TKN_ --token-policy single   \
		 --no-warning-on-no-token-queue

clean:
	rm -rf $(ENGINE)
	rm -rf $(ENGINE:.cpp=.o)
	rm -rf $(ENGINE:.cpp=)
	rm -rf $(ENGINE:.cpp=-token_ids)
	rm -rf $(ENGINE:.cpp=-token)
	rm -rf $(ENGINE:.cpp=-configuration)
	rm -rf lexer
	rm -rf *.E
	rm -rf lexer.o
