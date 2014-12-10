
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = spitfire
spitfire_SOURCES = src/Alliance.cpp src/Alliance.h src/AllianceCore.cpp src/AllianceCore.h src/funcs.cpp src/funcs.h src/City.cpp src/City.h src/amf3.cpp src/amf3.h src/Client.cpp src/Client.h src/combat.cpp src/combat.h src/main.cpp src/Hero.cpp src/Hero.h src/Map.cpp src/Map.h src/request_handler.cpp src/request_handler.h src/Server.cpp src/Server.h src/structs.h src/Tile.cpp src/Tile.h


spitfire_CPPFLAGS = -std=gnu++11 -Wno-write-strings -Wno-narrowing -Wno-conversion-null
spitfire_LDFLAGS = -L../boost/stage/lib
spitfire_LDADD = -llua5.2 -lPocoData -lPocoFoundation -lPocoMySQL -lPocoSQLite -lboost_chrono -lboost_date_time -lboost_system -lboost_thread -lpthread
