
Source_Dirs = Cpp Cpp/BackEnd Cpp/Docker Cpp/Lexer Cpp/Nodes Cpp/Parser Cpp/PreProsessor Cpp/UI Tests Tests/Cpp

Cpp_Sources = $(foreach dir,$(Source_Dirs),$(wildcard $(dir)/*.cpp))

Cpp_Objects = $(patsubst %.cpp,out/%.o,$(Cpp_Sources))

default: Evie

Debuggarna debug:
	$(MAKE) -Bj CFLAGS="$(CFLAGS) -g3"

Evie evie: $(Cpp_Objects)
	$(CXX) --std=c++2a $^ -o Evie -ldl -lstdc++ -lcurl -lpthread $(LDFLAGS)

clean:
	rm $(Cpp_Objects) Evie

out/%.o: %.cpp
	@mkdir -p $(basename $@)
	$(CXX) --std=c++2a -fpermissive -c $< -o $@ $(CFLAGS)
	