#include "header/expander.h"
#include "header/structs/myfx/Order.h"


/**
 *
 */
BOOL WINAPI CollectTestData(EXECUTION_CONTEXT* ec, datetime startTime, datetime endTime, double bid, double ask, uint bars, double accountBalance, const char* accountCurrency, const char* reportSymbol) {
   if ((uint)ec              < MIN_VALID_POINTER)  return(error(ERR_INVALID_PARAMETER, "invalid parameter ec=0x%p (not a valid pointer)", ec));
   if ((uint)accountCurrency < MIN_VALID_POINTER)  return(error(ERR_INVALID_PARAMETER, "invalid parameter accountCurrency=0x%p (not a valid pointer)", accountCurrency));
   if ((uint)reportSymbol    < MIN_VALID_POINTER)  return(error(ERR_INVALID_PARAMETER, "invalid parameter reportSymbol=0x%p (not a valid pointer)", reportSymbol));
   if (!ec->programId)                             return(error(ERR_INVALID_PARAMETER, "invalid execution context:  ec.programId=%d", ec->programId));
   if (ec->programType!=PT_EXPERT || !ec->testing) return(error(ERR_FUNC_NOT_ALLOWED, "function allowed only in experts under test"));

   TEST* test;

   if (ec->rootFunction == RF_START) {
      if (ec->test) return(error(ERR_RUNTIME_ERROR, "multiple TEST initializations in %s::start()", ec->programName));

      ec->test = test = new TEST();
      test_SetTime           (test, time(NULL)      );
      test_SetDuration       (test, GetTickCount()  );
      test_SetSymbol         (test, ec->symbol      );
      test_SetTimeframe      (test, ec->timeframe   );
      test_SetStartTime      (test, startTime       );
      //uint tickModel;                                              // TODO
      test_SetSpread         (test, (ask-bid)/0.0001);               // TODO: statt 0.0001 Variable Pip
      test_SetBars           (test, bars            );
      test_SetAccountDeposit (test, accountBalance  );               //  oder aus Expert.ini auslesen
      test_SetAccountCurrency(test, accountCurrency );               //  oder aus Expert.ini auslesen
      //uint tradeDirections;                                        // TODO: aus Expert.ini auslesen
      test_SetStrategy       (test, ec->programName );
      test->orders = new OrderHistory(512);                          // reserve memory to speed-up testing
      test->orders->resize(0);
   }
   else if (ec->rootFunction == RF_DEINIT) {
      test = ec->test;
      if (!test) return(error(ERR_RUNTIME_ERROR, "missing TEST initialization in %s::deinit()", ec->programName));

      test_SetDuration    (test, GetTickCount() - test->duration);
      test_SetEndTime     (test, endTime                        );
      test_SetBars        (test, bars - test->bars + 1          );
      test_SetTicks       (test, ec->ticks                      );
      test_SetReportSymbol(test, reportSymbol                   );
   }
   else return(error(ERR_FUNC_NOT_ALLOWED, "function not allowed in %s::%s()", ec->programName, RootFunctionDescription(ec->rootFunction)));

   debug("%s::%s()  test=%s", ec->programName, RootFunctionDescription(ec->rootFunction), TEST_toStr(test));
   return(TRUE);
   #pragma EXPORT
}


/**
 *
 */
BOOL WINAPI Test_OpenOrder(EXECUTION_CONTEXT* ec, int ticket, int type, double lots, const char* symbol, double openPrice, datetime openTime, double stopLoss, double takeProfit, double commission, int magicNumber, const char* comment) {
   if ((uint)ec < MIN_VALID_POINTER)                 return(error(ERR_INVALID_PARAMETER, "invalid parameter ec=0x%p (not a valid pointer)", ec));
   if (ec->programType!=PT_EXPERT || !ec->testing)   return(error(ERR_FUNC_NOT_ALLOWED, "function allowed only in experts under test"));

   TEST*         test   = ec->test;     if (!test)   return(error(ERR_RUNTIME_ERROR, "invalid TEST initialization,  ec.test=0x%p", ec->test));
   OrderHistory* orders = test->orders; if (!orders) return(error(ERR_RUNTIME_ERROR, "invalid OrderHistory initialization,  test.orders=0x%p", test->orders));

   ORDER order = {};
      order.ticket      = ticket;
      order.type        = type;
      order.lots        = round(lots, 2);
      strcpy(order.symbol, symbol);
      order.openPrice   = round(openPrice, 5);
      order.openTime    = openTime;
      order.stopLoss    = round(stopLoss,   5);
      order.takeProfit  = round(takeProfit, 5);
      order.commission  = round(commission, 2);
      order.magicNumber = magicNumber;
      strcpy(order.comment, comment);
   orders->push_back(order);

   debug("");
   return(TRUE);
   #pragma EXPORT
}


/**
 * @param  int      ticket
 * @param  double   closePrice
 * @param  datetime closeTime
 * @param  double   swap
 * @param  double   profit
 *
 * @return BOOL - success status
 */
BOOL WINAPI Test_CloseOrder(EXECUTION_CONTEXT* ec, int ticket, double closePrice, datetime closeTime, double swap, double profit) {
   if ((uint)ec < MIN_VALID_POINTER)                 return(error(ERR_INVALID_PARAMETER, "invalid parameter ec=0x%p (not a valid pointer)", ec));
   if (ec->programType!=PT_EXPERT || !ec->testing)   return(error(ERR_FUNC_NOT_ALLOWED, "function allowed only in experts under test"));

   TEST*         test   = ec->test;     if (!test)   return(error(ERR_RUNTIME_ERROR, "invalid TEST initialization,  ec.test=0x%p", ec->test));
   OrderHistory* orders = test->orders; if (!orders) return(error(ERR_RUNTIME_ERROR, "invalid OrderHistory initialization,  test.orders=0x%p", test->orders));

   uint i = orders->size()-1;

   for (; i >= 0; --i) {                                             // iterate in reverse order to speed-up
      ORDER* order = &(*orders)[i];
      if (order->ticket == ticket) {
         order->closePrice = round(closePrice, 5);
         order->closeTime  = closeTime;
         order->swap       = round(swap,   2);
         order->profit     = round(profit, 2);

         debug("order=%s", ORDER_toStr(order));
         break;
      }
   }
   if (i < 0) return(error(ERR_RUNTIME_ERROR, "ticket #%d not found, size(orders)=%d", ticket, orders->size()));

   return(TRUE);
   #pragma EXPORT
}