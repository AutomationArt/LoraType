
sym = checkButton();

if (sym != '~')
{
	symHistory(sym);
	if ((isIndChat && ((int)sym >= 32)) || (isGenChat && ((int)sym >= 32)))
	{
		outMessage += sym;
		chatDrawOutmess();
		drawUpdate();
	}

	if (sym == SPACE)
	{
		outMessage += " ";
		chatDrawOutmess();
		drawUpdate();
	}

	if (isGenChat && sym == ENTER)
	{
#ifdef logc
		Serial.println("isGenChat && sym == ENTER");
#endif
		if (outMessage.length() >= 3)
		{
			if (!loraSendBroadcast(outMessage))
			{
				outMessage = "";
				LedLoraWarning(true);
				pt.DrawFilledRectangle(3, 181, 197, 197, UNCOLORED);
				chatGenDraw();
			}
			else
			{
				LedLoraWarning(false);
			}

			MenuHistory(8);
			counterResMess = 0;
			drawUpdate();
		}
		else
		{

			LedSystemWarning();
		}
	}

	/*перемещение в основном меню*/

	// 		if (sym == UP && menuLast == 0 && menuNowSelect > 0)
	// 		{
	// 			menuNowSelect -= 1;
	// #ifdef logc
	// 			Serial.println(menuNowSelect);
	// #endif
	// 			MenuDrawArow(menuNowSelect);
	// 			drawUpdate();
	// 		}

	// 		if (sym == DOWN && menuLast == 0 && menuNowSelect <= menuCount - 1)
	// 		{
	// 			menuNowSelect += 1;
	// #ifdef logc
	// 			Serial.println(menuNowSelect);
	// #endif
	// 			MenuDrawArow(menuNowSelect);
	// 			drawUpdate();
	// 		}

	/* Выбор в главном меню */

	/*============Написать всем=============================================== */

	// if (sym == OK && menuLast == 0 && menuNowSelect == 0)
	// {
	// 	MenuHistory(0);
	// 	symHistory(sym);
	// 	String headerName = String(menuAll[menuNowSelect].itemName);
	// 	MenuHeader(headerName);
	// 	isIndChat = false;
	// 	isGenChat = true;
	// 	chatGenDraw();
	// 	drawUpdate();
	// 	sym = '~';
	// }

	// if (sym == UP && isGenChat)
	// {
	// 	if (messFrame < countinskStr && (allstrCounter - 10) > 1)
	// 	{
	// 		messFrame++;
	// 		chatGenDraw(messFrame);
	// 		drawUpdate();
	// 	}
	// 	sym = '~';
	// }

	// if (sym == DOWN && isGenChat)
	// {
	// 	// if (messFrame > 0)
	// 	// {
	// 	// 	messFrame--;
	// 	// 	chatGenDraw(messFrame);
	// 	// 	drawUpdate();
	// 	// }
	// 	sym = '~';
	// }

	/*==================Все юзеры======================================== */

	// All Users
	// if (sym == OK && menuLast == 0 && menuNowSelect == 1)
	// {
	// 	// MenuHistory(1);
	// 	// symHistory(sym);
	// 	// String headerName = String(menuAll[menuNowSelect].itemName);
	// 	// MenuHeader(headerName);
	// 	// MenuAllUserDraw();
	// 	// if (numElements > 0)
	// 	// {
	// 	// 	MenuDrawArow(0);
	// 	// }
	// 	// drawUpdate();
	// 	// menuNowSelect = 0;
	// 	// sym = '~';
	// }

	/* Перемещение стрелки в меню всех юзеров */
	if (sym == UP && menuLast == 1 && menuNowSelect >= 0)
	{
		menuNowSelect -= 1;
#ifdef logc
		Serial.println(menuNowSelect);
#endif								
		MenuDrawArow(menuNowSelect);
		drawUpdate();
	}
	if (sym == DOWN && menuLast == 1 && menuNowSelect <= menuCount - 1)
	{
		menuNowSelect += 1;
#ifdef logc
		Serial.println(menuNowSelect);
#endif
		MenuDrawArow(menuNowSelect);
		drawUpdate();
	}

	// Вход в меню юзеров
	if (sym == OK && menuLast == 1)
	{
		isGenChat = false;
		isIndChat = true; // старт индивидуального чата
		MenuHistory(7);
		symHistory(sym);
		getActiveUser = nodeId[menuNowSelect];
		MenuHeader(String(getActiveUser, HEX));
		chatSingleDraw(getActiveUser);
		drawUpdate();
	}

	// Отправка сообщения конкретному юзеру
	if (isIndChat && sym == ENTER)
	{
		if (outMessage.length() >= 3)
		{
			loraSendMessage(getActiveUser, outMessage);
			outMessage = "";
			counterResMess = 0;
			pt.DrawFilledRectangle(3, 181, 197, 197, UNCOLORED);
			chatSingleDraw(getActiveUser);
			MenuHistory(7);
			drawUpdate();
		}
	}

	/*===========All tags========================================== */

	if (sym == OK && menuLast == 0 && menuNowSelect == 2)
	{
		MenuHistory(2);
		symHistory(sym);
		isGenChat = false;
		isIndChat = false;

		String headerName = String(menuAll[menuNowSelect].itemName);
		MenuHeader(headerName);
		MenuAllTagsDraw();
		drawUpdate();
		menuNowSelect = 0;
		sym = '~';
	}

	if (sym == UP && menuLast == 2 && menuNowSelect >= 0)
	{
		menuNowSelect -= 1;
#ifdef logc
		Serial.println("menuNowSelect");
		Serial.println(menuNowSelect);
#endif
		MenuDrawArow(menuNowSelect);
		drawUpdate();
	}

	if (sym == DOWN && menuLast == 2 && menuNowSelect <= menuCount - 1)
	{
		menuNowSelect += 1;
#ifdef logc
		Serial.println("menuNowSelect");
		Serial.println(menuNowSelect);
#endif
		MenuDrawArow(menuNowSelect);
		drawUpdate();
	}

	if (sym == UP && isTagsView)
	{
		if (messFrame < countinskStr && (allstrCounter - 10) > 1)
		{
			messFrame++;
#ifdef logc
			Serial.print("U messFrame  ");
			Serial.println(messFrame);
#endif
			tagSingleDraw(getActiveTag, messFrame);
			drawUpdate();
		}
		sym = '~';
	}

	if (sym == DOWN && isTagsView)
	{
		if (messFrame > 0)
		{
			messFrame--;
#ifdef logc
			Serial.print("D messFrame  ");
			Serial.println(messFrame);
#endif

			tagSingleDraw(getActiveTag, messFrame);
			drawUpdate();
		}
		sym = '~';
	}

	if (sym == OK && menuLast == 2)
	{
#ifdef logc
		Serial.println("(sym == OK' && menuLast == 2)");
#endif
		MenuHistory(9);
		symHistory(sym);
		isTagsView = true;
		getActiveTag = AllTag[menuNowSelect].tag;

		counterResMess = 0;
		String headerName = String(menuAll[menuNowSelect].itemName);
		MenuHeader(headerName);
		tagSingleDraw(getActiveTag);
		drawUpdate();
		menuNowSelect = 0;

		drawUpdate();
		sym = '~';
	}

	/*=====================================================*/

	// DB stats
	if (sym == OK && menuLast == 0 && menuNowSelect == 6)
	{
		MenuHistory(3);
		symHistory(sym);
		String headerName = String(menuAll[menuNowSelect].itemName);
		MenuHeader(headerName);
		drawAbout();
		drawUpdate();
		sym = '~';
#ifdef logc
		DrawHistory();
#endif
	}

	// Lora settings
	if (sym == OK && menuLast == 0 && menuNowSelect == 3)
	{
		MenuHistory(4);
		symHistory(sym);
		String headerName = String(menuAll[menuNowSelect].itemName);
		MenuHeader(headerName);
		MenuDrawLoraSet();
		drawUpdate();
		sym = '~';
#ifdef logc
		DrawHistory();
#endif
	}

	/// Quality
	if (sym == OK && menuLast == 0 && menuNowSelect == 4)
	{
		MenuHistory(5);
		symHistory(sym);
		String headerName = String(menuAll[menuNowSelect].itemName);
		MenuHeader(headerName);
		MenuDrawDistance();
		drawUpdate();
		sym = '~';
	}

	// Update firmware
// 	if (sym == OK && menuLast == 0 && menuNowSelect == 5)
// 	{

// 		MenuHistory(6);
// 		symHistory(sym);

// 		String headerName = String(menuAll[menuNowSelect].itemName);
// 		MenuHeader(headerName);
// 		menuNowSelect = 0;
// 		pref.begin("Update", false);
// 		pref.putBool("state", true);
// 		pref.end();
// #ifdef logc
// 		Serial.println("State Update = true");
// #endif

// 		drawUpdate();
// 		delay(1000);
// 		ESP.restart();
// 		sym = '~';
// 	}

	// if (sym == BACKSPACE)
	// {
	// 	outMessage.remove(outMessage.length() - 1, 1);
	// 	chatDrawOutmess();
	// }	

	sym = '~';
}
