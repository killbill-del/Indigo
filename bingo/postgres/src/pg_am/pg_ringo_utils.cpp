extern "C" {
#include "postgres.h"
#include "fmgr.h"
}

#ifdef qsort
#undef qsort
#endif
#include "bingo_postgres.h"
#include "bingo_pg_text.h"
#include "bingo_core_c.h"
#include "bingo_pg_common.h"


extern "C" {
PG_FUNCTION_INFO_V1(aam);
PGDLLEXPORT Datum aam(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(rxnfile);
PGDLLEXPORT Datum rxnfile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(rcml);
PGDLLEXPORT Datum rcml(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(checkreaction);
PGDLLEXPORT Datum checkreaction(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(rsmiles);
PGDLLEXPORT Datum rsmiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(rfingerprint);
PGDLLEXPORT Datum rfingerprint(PG_FUNCTION_ARGS);
}

Datum aam(PG_FUNCTION_ARGS) {
   Datum react_datum = PG_GETARG_DATUM(0);
   Datum mode_datum = PG_GETARG_DATUM(1);
   
   char* result = 0;
   
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("aam");

      BingoPgText react_text(react_datum);
      BingoPgText aam_mode(mode_datum);

      int buf_size;
      const char* react_buf = react_text.getText(buf_size);
      const char* bingo_result = ringoAAM(react_buf, buf_size, aam_mode.getString());

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.AAM", bingoGetError());
         PG_RETURN_NULL();
      }

      result = BingoPgCommon::releaseString(bingo_result);
      
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum rxnfile(PG_FUNCTION_ARGS) {
   Datum react_datum = PG_GETARG_DATUM(0);

   char* result = 0;
   PG_BINGO_BEGIN
   {

      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("rxnfile");

      BingoPgText react_text(react_datum);
      int buf_size;
      const char* react_buf = react_text.getText(buf_size);
      const char* bingo_result = ringoRxnfile(react_buf, buf_size);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.rxnfile", bingoGetError());
         PG_RETURN_NULL();
      }

      result = BingoPgCommon::releaseString(bingo_result);

   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum rcml(PG_FUNCTION_ARGS) {
   Datum react_datum = PG_GETARG_DATUM(0);

   char* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("rcml");

      BingoPgText react_text(react_datum);
      int buf_size;
      const char* react_buf = react_text.getText(buf_size);
      const char* bingo_result = ringoRCML(react_buf, buf_size);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.rcml", bingoGetError());
         PG_RETURN_NULL();
      }

      result = BingoPgCommon::releaseString(bingo_result);

   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum checkreaction(PG_FUNCTION_ARGS) {
   Datum react_datum = PG_GETARG_DATUM(0);

   char* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("checkreaction");

      BingoPgText react_text(react_datum);
      int buf_size;
      const char* react_buf = react_text.getText(buf_size);
      const char* bingo_result = ringoCheckReaction(react_buf, buf_size);
      if(bingo_result == 0)
         PG_RETURN_NULL();

      result = BingoPgCommon::releaseString(bingo_result);

   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum rsmiles(PG_FUNCTION_ARGS) {
   Datum react_datum = PG_GETARG_DATUM(0);

   char* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("rsmiles");

      BingoPgText react_text(react_datum);
      int buf_size;
      const char* react_buf = react_text.getText(buf_size);
      const char* bingo_result = ringoRSMILES(react_buf, buf_size);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.rsmiles", bingoGetError());
         PG_RETURN_NULL();
      }

      result = BingoPgCommon::releaseString(bingo_result);
   }
   PG_BINGO_END

   if (result == 0)
      PG_RETURN_NULL();

   PG_RETURN_CSTRING(result);
}

Datum rfingerprint(PG_FUNCTION_ARGS){
   Datum react_datum = PG_GETARG_DATUM(0);
   Datum options_datum = PG_GETARG_DATUM(1);

   void* result = 0;
   PG_BINGO_BEGIN
   {
      BingoPgCommon::BingoSessionHandler bingo_handler(fcinfo->flinfo->fn_oid);
      bingo_handler.setFunctionName("rfingerprint");

      BingoPgText r_text(react_datum);
      BingoPgText react_options(options_datum);

      int buf_size;
      const char* mol_buf = r_text.getText(buf_size);

      int res_buf;
      const char* bingo_result = ringoFingerprint(mol_buf, buf_size, react_options.getString(), &res_buf);

      if(bingo_result == 0) {
         CORE_HANDLE_WARNING(0, 1, "bingo.rfingerprint", bingoGetError());
         PG_RETURN_NULL();
      }

      BingoPgText result_data;
      result_data.initFromBuffer(bingo_result, res_buf);

      result = result_data.release();
   }
   PG_BINGO_END

   if(result == 0)
      PG_RETURN_NULL();

   PG_RETURN_BYTEA_P(result);
}