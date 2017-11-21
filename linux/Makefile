#
# J1939Display Makefile
#

CC = gcc
LD = gcc

TCAN_API_DIR = /opt/tiny_can/dev/lib

OBJ_DIR = obj
OUTPUT_DIR = bin
PROGRAMM = J1939Display

SRC_DIR = ./../src
C_PREPROC = -DENABLE_NLS

CFLAGS = `pkg-config --cflags gtk+-2.0 libxml-2.0 libmodbus sqlite3` -Wall \
         -I./../src -I$(TCAN_API_DIR) -pthread -g0 -O2 #-ggdb3 -O0 

LIBS = `pkg-config --libs gtk+-2.0 libxml-2.0 libmodbus sqlite3` -ldl -lm
LDFLAGS = -pthread


SRC_OBJS = \
  $(OBJ_DIR)/can.o \
  $(OBJ_DIR)/can_autobaud.o \
  $(OBJ_DIR)/iconcache.o \
  $(OBJ_DIR)/mhs_file_event.o \
  $(OBJ_DIR)/mhs_g_messages.o \
  $(OBJ_DIR)/rrd_tool_database.o \
  $(OBJ_DIR)/msg_widget.o \
  $(OBJ_DIR)/modbus_io.o \
  $(OBJ_DIR)/start_win.o \
  $(OBJ_DIR)/cbuf.o \
  $(OBJ_DIR)/etable.o \
  $(OBJ_DIR)/gauge.o \
  $(OBJ_DIR)/gauge-private.o \
  $(OBJ_DIR)/gauge-xml.o \
  $(OBJ_DIR)/xmlbase.o \
  $(OBJ_DIR)/gauge_win.o \
  $(OBJ_DIR)/gtk-digital-show.o \
  $(OBJ_DIR)/gtk-ex-frame.o \
  $(OBJ_DIR)/gtk-lcd.o \
  $(OBJ_DIR)/lcd_win.o \
  $(OBJ_DIR)/lcd_gauge_win.o \
  $(OBJ_DIR)/main.o \
  $(OBJ_DIR)/trace.o \
  $(OBJ_DIR)/util.o \
  $(OBJ_DIR)/xml_database.o \
  $(OBJ_DIR)/sqlite_database.o \
  $(OBJ_DIR)/configfile.o \
  $(OBJ_DIR)/setup.o \
  $(OBJ_DIR)/can_drv_linux.o

all: create_dirs create_app

create_dirs:
	mkdir -p "$(OBJ_DIR)"
	mkdir -p "$(OUTPUT_DIR)"

create_app: $(SRC_OBJS)
	$(LD) -o $(OUTPUT_DIR)/$(PROGRAMM) $(CFLAGS) $(SRC_OBJS) $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/can.o: $(SRC_DIR)/can.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/can_autobaud.o: $(SRC_DIR)/can_autobaud.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/iconcache.o: $(SRC_DIR)/iconcache.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/mhs_file_event.o: $(SRC_DIR)/mhs_file_event.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/mhs_g_messages.o: $(SRC_DIR)/mhs_g_messages.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/rrd_tool_database.o: $(SRC_DIR)/rrd_tool_database.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/msg_widget.o: $(SRC_DIR)/msg_widget.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/modbus_io.o: $(SRC_DIR)/modbus_io.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/start_win.o: $(SRC_DIR)/start_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/cbuf.o: $(SRC_DIR)/cbuf.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/etable.o: $(SRC_DIR)/etable.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gauge.o: $(SRC_DIR)/gauge.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gauge-private.o: $(SRC_DIR)/gauge-private.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gauge-xml.o: $(SRC_DIR)/gauge-xml.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/xmlbase.o: $(SRC_DIR)/xmlbase.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gauge_win.o: $(SRC_DIR)/gauge_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gtk-digital-show.o: $(SRC_DIR)/gtk-digital-show.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gtk-ex-frame.o: $(SRC_DIR)/gtk-ex-frame.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gtk-lcd.o: $(SRC_DIR)/gtk-lcd.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/lcd_win.o: $(SRC_DIR)/lcd_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/lcd_gauge_win.o: $(SRC_DIR)/lcd_gauge_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/trace.o: $(SRC_DIR)/trace.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/util.o: $(SRC_DIR)/util.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/xml_database.o: $(SRC_DIR)/xml_database.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/sqlite_database.o: $(SRC_DIR)/sqlite_database.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@  

$(OBJ_DIR)/configfile.o: $(SRC_DIR)/configfile.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/setup.o: $(SRC_DIR)/setup.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/can_drv_linux.o: $(TCAN_API_DIR)/can_drv_linux.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:	
	rm -f *.o 
	rm -f *~ 
	rm -f ./$(OBJ_DIR)/*.o
	rm -f ./$(OBJ_DIR)/*~

