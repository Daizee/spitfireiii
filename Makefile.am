
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

bin_PROGRAMS = spitfire
spitfire_SOURCES = src/amf3.cpp src/funcs.cpp src/main.cpp


spitfire_CPPFLAGS = -std=gnu++11 -Wno-write-strings -Wno-narrowing -Wno-conversion-null
spitfire_LDFLAGS = -L../boost/stage/lib
spitfire_LDADD = -llua5.2 -lPocoData -lPocoFoundation -lPocoMySQL -lPocoSQLite -lboost_chrono -lboost_date_time -lboost_system -lboost_thread -lpthread
