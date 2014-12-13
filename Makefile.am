
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = spitfire
spitfire_SOURCES =  src/amf3.cpp src/funcs.cpp src/main.cpp src/Server.cpp src/connection.cpp \
					src/Alliance.cpp src/AllianceCore.cpp src/City.cpp src/Client.cpp src/combat.cpp \
					src/Hero.cpp src/Map.cpp src/reply.cpp src/request_handler.cpp src/Tile.cpp \
					src/packets/palliance.cpp src/packets/parmy.cpp src/packets/pcastle.cpp \
					src/packets/pcity.cpp src/packets/pcommon.cpp src/packets/pfield.cpp \
					src/packets/pfortifications.cpp src/packets/pfriend.cpp src/packets/pfurlough.cpp \
					src/packets/pgameclient.cpp src/packets/phero.cpp src/packets/pinterior.cpp \
					src/packets/plogin.cpp src/packets/packet.cpp src/packets/pmail.cpp \
					src/packets/pquest.cpp src/packets/prank.cpp src/packets/preport.cpp \
					src/packets/pserver.cpp src/packets/pshop.cpp src/packets/ptech.cpp \
					src/packets/ptrade.cpp src/packets/ptroop.cpp src/packets/punknown.cpp


spitfire_CPPFLAGS = -std=gnu++11 -Wno-write-strings -Wno-narrowing -Wno-conversion-null
spitfire_LDFLAGS = -L../boost/stage/lib
spitfire_LDADD = -llua5.2 -lPocoData -lPocoFoundation -lPocoMySQL -lPocoSQLite -lboost_chrono -lboost_date_time -lboost_system -lboost_thread -lpthread
