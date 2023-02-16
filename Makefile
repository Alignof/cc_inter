INCLUDE = -I./include -I/usr/include
TARGET  := ./cc_inter
SRCDIR  := ./src
OBJDIR  := ./src/obj
SOURCES := $(wildcard ./src/*.c)
OBJECTS := $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.c=.o)))

$(TARGET): $(OBJECTS)
	echo $(TARGET_ARCH)
	echo $(BT)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c 
	@[ -d $(OBJDIR) ]
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<


install: $(OBJECTS)
	$(CC) -O2 -o $(TARGET) $^ $(LDFLAGS)

test: $(TARGET)
	./test.sh $(TARGET_ARCH)

file_test: $(TARGET)
	$(TARGET) test.c > tmp.s && $(BT) -static tmp.s -o tmp
	./tmp || echo $$?

gcc_test: 
	$(BT) test.c -S -masm=intel -O0 -o tmp.s && $(BT) -static -O0 tmp.s -o tmp
	./tmp || echo $$?


self_host: $(TARGET)
	# prepare
	rm -rf self_host/
	mkdir self_host/
	cp src/*.c self_host/
	cp include/$(TARGET_ARCH)/cc_inter.h self_host/
	cat self_host/cc_inter.h > self_host.c
ifeq ($(TARGET_ARCH),x8664)
	cat `ls --ignore=codegen_riscv.c -F src/ | grep -v / | perl -pe 's//src\//'` >> self_host.c
else
	cat `ls --ignore=codegen_x8664.c -F src/ | grep -v / | perl -pe 's//src\//'` >> self_host.c
endif
	rm -rf self_host/

	# gen1
	$(TARGET) self_host.c > child.s 
	$(BT) -static child.s -o child
	cp child.s gen1.s
	# gen2
	$(SPIKE) $(PK) ./child self_host.c > child.s
	perl -pi -e 's/^bbl loader\r\n//' child.s
	$(BT) -static child.s -o child
	cp child.s gen2.s
	# gen3
	$(SPIKE) $(PK) ./child self_host.c > child.s
	perl -pi -e 's/^bbl loader\r\n//' child.s
	$(BT) -static child.s -o child
	cp child.s gen3.s

	# check
	diff gen2.s gen3.s

clean:
	rm -f cc_inter *.o *.s *~ tmp* *.txt *.out child* gen*
	rm -f $(OBJECTS) $(TARGET)

.PHONY: test clean install self_host
