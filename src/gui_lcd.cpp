#include "gui_lcd.h"
#include "esp_adc_cal.h"

#include <HTTPClient.h>
#include <ESP32httpUpdate.h>
#include "AiEsp32RotaryEncoder.h"

#define SerialLOG Serial

unsigned long dimTimeout = 0;
unsigned long currentTime;
unsigned long loopTime;
int encoder0Pos = 0;
int encoder0PosPrev = 0;
unsigned char encoder_A = 0;
unsigned char encoder_A_prev = 0;
char curTab = 0;
int posNow = 0;
int timeHalfSec = 0;
int line = 16;

uint8_t gps_mode = 0;

#define ROTARY_ENCODER_A_PIN 47
#define ROTARY_ENCODER_B_PIN 46
#define ROTARY_ENCODER_BUTTON_PIN 21

#define ROTARY_ENCODER_STEPS 4
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS);

void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}

const char *str_status[] = {
    "IDLE_STATUS",
    "NO_SSID_AVAIL",
    "SCAN_COMPLETED",
    "CONNECTED",
    "CONNECT_FAILED",
    "CONNECTION_LOST",
    "DISCONNECTED"};

void MyTextBox::TextBox()
{
    int w = (length * 7) + 4;
    ;
    int char_with = 7;
    int i;
    bool ok = false;
    display.fillRect(x, y, w, 11, BLACK);
    display.drawRect(x, y, w, 11, WHITE);
    curr_cursor = strlen(text);
    if (curr_cursor > 0)
    {
        curr_cursor--;
        encoder0Pos = text[curr_cursor];
    }
    else
    {
        encoder0Pos = 0x30; // 0x20-0x7F
    }

    do
    {
        if (type == 0)
        {
            char_min = 0x20;
            char_max = 0x7F;
        }
        else if (type == 1)
        {
            char_min = 0x2B;
            char_max = 0x39;
        }
        else if (type == 2)
        {
            char_min = 0x41;
            char_max = 0x5A;
        }

        if (encoder0Pos > char_max)
            encoder0Pos = char_min;
        if (encoder0Pos < char_min)
            encoder0Pos = char_max;

        display.fillRect(x + 1, y + 1, w - 2, 9, BLACK);
        for (i = 0; i <= (int)strlen(text); i++)
        {
            if (curr_cursor == i)
            {
                display.fillRect((i * char_with) + x + 1, y, 7, 10, WHITE);
                display.setTextColor(BLACK);
            }
            display.setCursor((i * char_with) + x + 2, y + 2);
            display.print(text[i]);
            display.setTextColor(WHITE);
        }
        text[curr_cursor] = encoder0Pos;
        display.display();
        delay(50);
        if ((digitalRead(keyPush) == LOW))
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                if ((millis() - currentTime) > 2000)
                    break; // OK Timeout
            };
            if ((millis() - currentTime) < 1500)
            {
                delay(100);
                currentTime = millis();
                while (digitalRead(keyPush) == HIGH)
                {
                    if ((millis() - currentTime) > 1000)
                        break; // OK Timeout
                };
                if ((millis() - currentTime) < 1000)
                { // Duble Click
                    // text[curr_cursor] = 0;
                    // for (i = curr_cursor; i < sizeof(text); i++) text[i] = 0;
                    if (curr_cursor == 0)
                    {
                        ok = true;
                        memset(text, 0, sizeof(text));
                    }
                    if (curr_cursor > 0)
                        curr_cursor--;
                    encoder0Pos = text[curr_cursor];
                }
                else
                { // One Click
                    if (curr_cursor < (length - 1))
                        curr_cursor++;
                    else
                        curr_cursor = length - 1;
                    // text[curr_cursor] = 0;
                }
                for (i = curr_cursor; i < 50; i++)
                    text[i] = 0;
                while (digitalRead(keyPush) == LOW)
                    ;
            }
            else
            {
                ok = true;
            }
        }
    } while (!ok);
    display.fillRect(x + 1, y + 1, w - 2, 9, BLACK);
    for (i = 0; i <= curr_cursor; i++)
    {
        display.drawChar((i * char_with) + x + 2, y + 2, text[i], BLACK, WHITE, 1);
        /*display.setCursor((i * char_with) + x + 2, y + 2);
        display.print(text[i]);*/
    }
    text[i] = 0;
    display.display();
    // msgBox(F("KEY Back"));
    while (digitalRead(keyPush) == LOW)
        ;
}

void MyTextBox::TextBoxShow()
{
    int w = (length * 7) + 4;
    int char_with = 7;
    int i;
    display.fillRect(x, y, w, 11, BLACK);
    display.drawRect(x, y, w, 11, WHITE);
    display.fillRect(x + 1, y + 1, w - 2, 9, BLACK);
    if (isSelect)
        display.drawRect(x + 1, y + 1, w - 2, 9, WHITE);
    for (i = 0; i <= (int)strlen(text); i++)
    {
        // for (i = 0; i <= curr_cursor; i++) {
        if (i >= length)
            break;
        display.drawChar((i * char_with) + x + 2, y + 2, text[i], WHITE, BLACK, 1);
        /*display.setCursor((i * char_with) + x + 2, y + 2);
        display.print(text[i]);*/
    }
    display.display();
}

void MyCheckBox::Toggle()
{
    if (Checked)
        Checked = false;
    else
        Checked = true;
}

void MyCheckBox::CheckBoxShow()
{
    int w = (strlen(text) * 6) + 10;
    // int margin_x = 1, char_with = 7;
    // int i;
    display.fillRect(x, y, w, 8, BLACK);
    display.drawRect(x, y, 8, 8, WHITE);
    display.fillRect(x + 1, y + 1, 6, 6, BLACK);
    if (Checked)
    {
        display.drawLine(x + 1, y + 1, x + 7, y + 7, WHITE);
        display.drawLine(x, y + 7, x + 7, y + 1, WHITE);
    }
    display.setCursor(x + 10, y);
    display.print(text);
    if (isSelect)
    {
        display.setCursor(x + 11, y + 1);
        display.print(text);
    }
    display.display();
}

void MyButtonBox::Toggle()
{
    if (Checked)
        Checked = false;
    else
        Checked = true;
}

void MyButtonBox::Show()
{
    int w = (strlen(text) * 6) + 4;
    // int w = (length * 7) + 4;
    // int char_with = 7;
    // unsigned int i;
    display.fillRect(x, y, w, 12, BLACK);
    if (isSelect)
    {
        // if (Border) {
        // display.fillRect(x, y, w, 11, WHITE);
        /*display.drawLine(x + 1, y + 12, x + w + 1, y + 12, WHITE);
        display.drawLine(x + w + 1, y + 1, x + w + 1, y + 12, WHITE);*/
        //}
        display.fillRect(x, y, w, 11, WHITE);
        display.setTextColor(BLACK);
        display.setCursor(x + 2, y + 2);
        display.print(text);
    }
    else
    {
        if (Border)
        {
            display.drawRect(x, y, w, 11, WHITE);
            display.drawLine(x + 1, y + 11, x + w, y + 11, WHITE);
            display.drawLine(x + w, y + 1, x + w, y + 11, WHITE);
        }
        display.setTextColor(WHITE);
        display.setCursor(x + 2, y + 2);
        display.print(text);
    }
    display.setTextColor(WHITE);
}

void MyComboBox::SelectValue(long val_min, long val_max, long step)
{
    int w = (length * 7) + 4;
    int keyPrev = encoder0Pos;
    bool ok = false;
    display.fillRect(x, y, w, 11, BLACK);
    display.drawRect(x, y, w, 11, WHITE);

    // current = atol(text);
    if (current > val_max)
        current = val_min;

    display.fillRect(x + 1, y + 1, w - 2, 9, WHITE);
    display.setTextColor(BLACK);

    display.setCursor(x + 2, y + 2);
    display.print(current, DEC);
    display.setTextColor(WHITE);
    display.display();

    do
    {

        if (encoder0Pos != keyPrev)
        {
            if (encoder0Pos < keyPrev)
            {
                current -= step;
            }
            else if (encoder0Pos > keyPrev)
            {
                current += step;
            }
            if (current > val_max)
                current = val_min;
            if (current < val_min)
                current = val_max;

            keyPrev = encoder0Pos;
            // sprintf(text, "%l", current);

            display.fillRect(x + 1, y + 1, w - 2, 9, WHITE);
            display.setTextColor(BLACK);
            /*for (i = 0; i <= strlen(text); i++) {
                display.setCursor((i * char_with) + x + 2, y + 2);
                display.print(text[i]);
            }*/
            display.setCursor(x + 2, y + 2);
            display.print(current, DEC);
            display.setTextColor(WHITE);
            display.display();
        }

        delay(50);
        if ((digitalRead(keyPush) == LOW))
        {
            currentTime = millis();
            delay(500);
            // if (digitalRead(keyPush) == HIGH)
            ok = true;
        }
    } while (!ok);
    display.fillRect(x + 1, y + 1, w - 2, 9, BLACK);
    display.setCursor(x + 2, y + 2);
    display.print(current, DEC);
    // text[i] = 0;
    display.display();
    // msgBox(F("KEY Back"));
    while (digitalRead(keyPush) == LOW)
        ;
}

void MyComboBox::AddItem(int index, char *str)
{
    strcpy(&item[index][0], str);
}

void MyComboBox::GetItem(int index, char *str)
{
    strcpy(str, &item[index][0]);
}

void MyComboBox::maxItem(unsigned char index)
{
    char_max = index;
}

unsigned long MyComboBox::GetValue()
{
    return current;
}

unsigned char MyComboBox::GetIndex()
{
    return current_index;
}
void MyComboBox::SetIndex(unsigned int i)
{
    if (isValue)
    {
        current = i;
    }
    else
    {
        current_index = i;
        if (current_index >= char_max)
            current_index = 0;
    }
}

void MyComboBox::SelectItem()
{
    int w = (length * 7) + 4;
    int keyPrev = encoder0Pos;
    bool ok = false;
    Show();
    display.fillRect(x + 1, y + 1, w - 2, 9, WHITE);
    display.setTextColor(BLACK);
    display.setCursor(x + 2, y + 2);
    display.print(item[current_index]);
    display.setTextColor(WHITE);
    display.display();

    do
    {
        if (encoder0Pos != keyPrev)
        {
            if (encoder0Pos < keyPrev)
            {
                current_index++;
            }
            else if (encoder0Pos > keyPrev)
            {
                current_index--;
            }
            if (current_index >= char_max)
                current_index = char_min;
            // if (current_index < char_min) current_index = char_max;

            keyPrev = encoder0Pos;
            // tb = (double)current / 1000000;
            // dtostrf(tb, 3, 5, text);

            display.fillRect(x + 1, y + 1, w - 2, 9, WHITE);
            display.setTextColor(BLACK);
            display.setCursor(x + 2, y + 2);
            display.print(item[current_index]);
            display.setTextColor(WHITE);
            display.display();
        }

        delay(50);
        if ((digitalRead(keyPush) == LOW))
        {
            currentTime = millis();
            delay(500);
            // if (digitalRead(keyPush) == HIGH)
            ok = true;
        }
    } while (!ok);
    Show();
    // msgBox(F("KEY Back"));
    while (digitalRead(keyPush) == LOW)
        ;
    // Show();
}

void MyComboBox::Show()
{
    int w = (length * 7) + 4;
    // int char_with = 7;
    // unsigned int i;
    display.fillRect(x, y, w + 10, 11, BLACK);
    display.drawRect(x, y, w, 11, WHITE);
    display.drawRect(x + w - 1, y, 10, 11, WHITE);
    // display.fillRect(x + 1, y + 1, w - 2, 9, BLACK);
    if (isSelect)
    {
        // display.drawRect(x + 1, y + 1, w - 2, 9, WHITE);
        // display.fillTriangle(x + w + 1, y + 9, x + w + 1 + 4, y + 2, x + w + 1 + 8, y + 9, WHITE);
        // display.fillTriangle(x + w + 2 + 5, y + 2, x + w + 2 + 5 + 8, y + 2, x + w + 2 + 5 + 4, y + 9, WHITE);
        display.fillTriangle(x + w, y + 2, x + w + 8, y + 2, x + w + 4, y + 9, WHITE);
    }
    else
    {
        // display.drawTriangle(x + w + 1, y + 9, x + w + 1 + 4, y + 2, x + w + 1 + 8, y + 9, WHITE);
        // display.drawTriangle(x + w + 2 + 5, y + 2, x + w + 2 + 5 + 8, y + 2, x + w + 2 + 5 + 4, y + 9, WHITE);
        display.drawTriangle(x + w, y + 2, x + w + 8, y + 2, x + w + 4, y + 9, WHITE);
    }
    if (isValue == true)
    {
        // sprintf(text, "%d", current);
        display.setCursor(x + 2, y + 2);
        display.print(current, DEC);
    }
    else
    {
        display.setCursor(x + 2, y + 2);
        display.print(item[current_index]);
        // for (i = 0; i <= strlen(item[current_index]); i++) {
        //	display.setCursor((i * char_with) + x + 2, y + 2);
        //	display.print(item[current_index][i]);
        // }
    }
    display.display();
}

unsigned char MySymbolBox::GetTable()
{
    return table;
}
unsigned char MySymbolBox::GetSymbol()
{
    return symbol;
}
unsigned char MySymbolBox::GetIndex()
{
    return MySymbolBox::current_index;
}
void MySymbolBox::SetIndex(unsigned char i)
{
    current_index = i;
    if (current_index >= 0x80)
        current_index = 0x80;
}

void MySymbolBox::SelectItem()
{
    int keyPrev = encoder0Pos;
    bool ok = false;
    onSelect = true;
    if (table == '/')
    {
        tableMode = 0;
    }
    else if (table == '\\')
    {
        tableMode = 1;
    }
    else
    {
        tableMode = 2;
    }

    Show();

    do
    {
        if (encoder0Pos != keyPrev)
        {
            if (encoder0Pos < keyPrev)
            {
                current_index++;
            }
            else if (encoder0Pos > keyPrev)
            {
                current_index--;
            }
            if (tableMode == 2)
            {
                if (current_index >= 'Z')
                    current_index = 'A';
                if (current_index < 'A')
                    current_index = 'Z';
            }
            else
            {
                if (current_index >= 0x80)
                    current_index = 0x21;
                if (current_index < 0x21)
                    current_index = 0x80;
            }

            keyPrev = encoder0Pos;
            if (tableMode == 2)
            {
                table = current_index;
                symbol = '&';
            }
            else
            {
                symbol = current_index;
            }
            Show();
        }

        delay(50);
        if ((digitalRead(keyPush) == LOW))
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                if ((millis() - currentTime) > 2000)
                    break; // OK Timeout
            };
            if ((millis() - currentTime) < 1500)
            {
                delay(100);
                // currentTime = millis();
                // while (digitalRead(keyPush) == HIGH) {
                //	delay(10);
                //	if ((millis() - currentTime) > 1000) break; //OK Timeout
                // };
                if (++tableMode > 2)
                    tableMode = 0;
                switch (tableMode)
                {
                case 0:
                    table = '/';
                    break;
                case 1:
                    table = '\\';
                    break;
                case 2:
                    if (table < 'A' || table > 'Z')
                        table = 'N';
                    symbol = '&';
                    break;
                }
                // if (table == '/')
                //	table = '\\';
                // else if (table == '\\')
                //	table = '/';
                Show();
                // while (digitalRead(keyPush) == LOW) delay(10);
            }
            else
            {
                ok = true;
            }
        }
    } while (!ok);
    onSelect = false;
    Show();
    // msgBox(F("KEY Back"));
    while (digitalRead(keyPush) == LOW)
        ;
    // Show();
}

void MySymbolBox::Show()
{
    // int w = 16 + 4;
    // int char_with = 7;
    // unsigned int i;
    display.fillRect(x, y, 20 + 14, 20, BLACK);
    // display.drawRect(x, y, 20, 20, WHITE);
    if (isSelect)
    {
        if (onSelect)
        {
            display.drawRoundRect(x, y, 20, 20, 5, WHITE);
        }
        else
        {
            display.drawRect(x, y, 20, 20, WHITE);
            display.drawRect(x + 1, y + 1, 18, 18, WHITE);
        }
        display.setCursor(x + 22, y + 2);
        display.print(table);
        display.setCursor(x + 22, y + 11);
        display.print(symbol);
    }
    else
    {
        display.drawRect(x, y, 20, 20, WHITE);
    }
    const uint8_t *ptrSymbol;
    uint8_t symIdx = symbol - 0x21;
    if (symIdx > 95)
        symIdx = 0;
    if (table == '/')
    {
        ptrSymbol = &Icon_TableA[symIdx][0];
    }
    else if (table == '\\')
    {
        ptrSymbol = &Icon_TableB[symIdx][0];
    }
    else
    {
        if (table < 'A' || table > 'Z')
            table = 'N';
        symbol = '&';
        symIdx = 5; // &
        ptrSymbol = &Icon_TableB[symIdx][0];
    }
    display.drawYBitmap(x + 2, y + 2, ptrSymbol, 16, 16, WHITE);
    if (!(table == '/' || table == '\\'))
    {
        display.drawChar(x + 7, y + 6, table, BLACK, WHITE, 1);
        display.drawChar(x + 8, y + 7, table, BLACK, WHITE, 1);
    }
    display.display();
}

// Renderer

class MyRenderer : public MenuComponentRenderer
{
public:
    virtual void render(Menu const &menu) const
    {
        String str;
        int x;
        display.clearDisplay();
        display.fillRect(0, 0, 128, 14, WHITE);
        // display.setFont(&FreeSansBold9pt7b);
        display.setFont(&FreeSerifItalic9pt7b);
        display.setTextColor(BLACK);
        str = String(menu.get_name());
        if (str.length() == 0)
            str = String("MAIN MENU");
        // x = str.length() * 6;
        // display.setCursor(64 - (x / 2), 4);
        x = str.length() * 10;
        display.setCursor(63 - (x / 2), 12);
        display.print(str);

        // display.setCursor(0, 8);
        // display.print(menu.get_current_component_num(),DEC);
        // display.setCursor(10, 8);
        // display.print(menu.get_num_components(), DEC);
        // display.setCursor(20, 8);
        // display.print(menu.get_previous_component_num(), DEC);
        // menu.render(*this);
        // menu.get_current_component()->render(*this);
        display.setFont();
        display.setTextColor(WHITE);
        int16_t line = 16;
        for (int i = 0; i < menu.get_num_components(); ++i)
        {
            MenuComponent const *cp_m_comp = menu.get_menu_component(i);
            cp_m_comp->render(*this);
            // SerialLOG.println(cp_m_comp->get_name());
            display.setCursor(10, line);
            display.print(cp_m_comp->get_name());

            if (cp_m_comp->is_current())
            {
                // SerialLOG.print("<<< ");
                display.setCursor(0, line);
                display.write(16);
            }
            line = line + 10;
            // SerialLOG.println("");
        }
        display.display();
    }

    virtual void render_menu_item(MenuItem const &menu_item) const
    {
        // display.fillRect(0, 8, 128, 8, BLACK);
        // display.setCursor(0, 8);
        // display.print(menu_item.get_name());
        // display.display();
    }

    virtual void render_back_menu_item(BackMenuItem const &menu_item) const
    {
        // display.fillRect(0, 8, 128, 8, BLACK);
        // display.setCursor(0, 1 * 8);
        // display.print(menu_item.get_name());
        // display.display();
    }

    virtual void render_numeric_menu_item(NumericMenuItem const &menu_item) const
    {
        // display.fillRect(0, 8, 128, 8, BLACK);
        // display.setCursor(0, 1 * 8);
        // display.print(menu_item.get_name());
    }

    virtual void render_menu(Menu const &menu) const
    {
        // display.fillRect(0, 0, 128, 8, BLACK);
        // display.setCursor(0, 0);
        // display.print(menu.get_name());
        // display.display();
    }
};

MyRenderer my_renderer;

void on_stationbeacon_selected(MenuItem *p_menu_item)
{
    int i;
    MyCheckBox chkBox[3];
    int max_sel = 8;
    MySymbolBox symBox[2];
    MyComboBox cbBox[3];
    String str;
    int x;
    int keyPrev = -1;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("STATION BEACON");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    chkBox[0].Checked = config.sta_compress;
    chkBox[0].x = 0;
    chkBox[0].y = 18;
    sprintf(chkBox[0].text, "COMP");

    chkBox[1].Checked = config.sta_altitude;
    chkBox[1].x = 40;
    chkBox[1].y = 18;
    sprintf(chkBox[1].text, "ALT");

    chkBox[2].Checked = config.sta_speed;
    chkBox[2].x = 75;
    chkBox[2].y = 18;
    sprintf(chkBox[2].text, "CSR/SPD");

    display.setCursor(0, 30);
    display.print("SPD:");
    cbBox[0].isValue = true;
    cbBox[0].x = 25;
    cbBox[0].y = 28;
    cbBox[0].length = 3;
    cbBox[0].char_max = 250;
    cbBox[0].SetIndex(config.sta_hspeed);

    display.setCursor(0, 42);
    display.print("INV:");
    cbBox[1].isValue = true;
    cbBox[1].x = 25;
    cbBox[1].y = 40;
    cbBox[1].length = 2;
    cbBox[1].char_max = 120;
    cbBox[1].SetIndex(config.sta_maxinterval);

    display.setCursor(0, 54);
    display.print("ANG:");
    cbBox[2].isValue = true;
    cbBox[2].x = 25;
    cbBox[2].y = 52;
    cbBox[2].length = 2;
    cbBox[2].char_max = 180;
    cbBox[2].SetIndex(config.sta_minangle);

    display.setCursor(67, 35);
    display.print("MOV");
    symBox[0].x = 65;
    symBox[0].y = 43;
    symBox[0].table = config.sta_symmove[0];
    symBox[0].symbol = config.sta_symmove[1];
    symBox[0].SetIndex(config.sta_symmove[1]);
    symBox[0].Show();

    display.setCursor(99, 35);
    display.print("STP");
    symBox[1].x = 98;
    symBox[1].y = 43;
    symBox[1].table = config.sta_symstop[0];
    symBox[1].symbol = config.sta_symstop[1];
    symBox[1].SetIndex(config.sta_symstop[1]);
    symBox[1].Show();

    display.display();
    encoder0Pos = 0;
    delay(100);
    do
    {
        if (encoder0Pos >= max_sel)
            encoder0Pos = 0;
        if (encoder0Pos < 0)
            encoder0Pos = max_sel - 1;
        if (keyPrev != encoder0Pos)
        {
            keyPrev = encoder0Pos;
            for (i = 0; i < 3; i++)
            {
                chkBox[i].isSelect = false;
                cbBox[i].isSelect = false;
            }
            symBox[0].isSelect = false;
            symBox[1].isSelect = false;

            if (encoder0Pos < 3)
                chkBox[encoder0Pos].isSelect = true;
            if (encoder0Pos > 2 && encoder0Pos < 6)
                cbBox[encoder0Pos - 3].isSelect = true;
            if (encoder0Pos > 5)
                symBox[encoder0Pos - 6].isSelect = true;
            for (i = 0; i < 3; i++)
            {
                chkBox[i].CheckBoxShow();
                cbBox[i].Show();
            }
            symBox[0].Show();
            symBox[1].Show();
        }
        else
        {
            delay(50);
        }
        if (digitalRead(keyPush) == LOW)
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                if ((millis() - currentTime) > 2000)
                    break; // OK Timeout
            };
            if ((millis() - currentTime) < 1500)
            {
                i = encoder0Pos;
                if (i < 3)
                {
                    chkBox[i].Toggle();
                    switch (i)
                    {
                    case 0:
                        config.sta_compress = chkBox[i].Checked;
                        break;
                    case 1:
                        config.sta_altitude = chkBox[i].Checked;
                        break;
                    case 2:
                        config.sta_speed = chkBox[i].Checked;
                        break;
                    }
                    encoder0Pos = keyPrev;
                    chkBox[i].CheckBoxShow();
                }
                else if (i > 2 && i < 6)
                {
                    i -= 3;
                    switch (i)
                    {
                    case 0:
                        cbBox[i].SelectValue(10, 200, 1);
                        config.sta_hspeed = cbBox[i].GetValue();
                        break;
                    case 1:
                        cbBox[i].SelectValue(5, 60, 1);
                        config.sta_maxinterval = cbBox[i].GetValue();
                        break;
                    case 2:
                        cbBox[i].SelectValue(5, 90, 1);
                        config.sta_minangle = cbBox[i].GetValue();
                        break;
                    }
                    encoder0Pos = keyPrev;
                    cbBox[i].Show();
                }
                else if (encoder0Pos > 5)
                {
                    i -= 6;
                    symBox[i].SelectItem();
                    switch (i)
                    {
                    case 0:
                        config.sta_symmove[0] = symBox[i].table;
                        config.sta_symmove[1] = symBox[i].symbol;
                        break;
                    case 1:
                        config.sta_symstop[0] = symBox[i].table;
                        config.sta_symstop[1] = symBox[i].symbol;
                        break;
                    }
                    encoder0Pos = keyPrev;
                    symBox[i].Show();
                }
                while (digitalRead(keyPush) == LOW)
                    ;
            }
            else
            {
                break;
            }
        }
    } while (1);
    /*display.clearDisplay();
    display.setCursor(30, 4);
    display.print("SAVE & EXIT");
    display.display();*/
    msgBox("KEY EXIT");
    while (digitalRead(keyPush) == LOW)
        ;
    saveEEPROM();
}

void on_wifi_selected(MenuItem *p_menu_item)
{
    MyTextBox txtBox[2];
    MyCheckBox chkBoxWiFi;
    MyComboBox cbBox;
    String str;
    // char ch[10];
    int x, i;
    int max_sel = 4;
    int keyPrev = -1;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("WIFI CONFIGURATION");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    display.setCursor(0, 18);
    display.print("SSID:");
    txtBox[0].x = 0;
    txtBox[0].y = 27;
    txtBox[0].length = 17;
    txtBox[0].type = 0;
    strcpy(txtBox[0].text, config.wifi_ssid);

    chkBoxWiFi.Checked = config.wifi;
    chkBoxWiFi.x = 92;
    chkBoxWiFi.y = 18;
    sprintf(chkBoxWiFi.text, "WiFi");

    display.setCursor(0, 44);
    display.print("PASS:");
    txtBox[1].x = 0;
    txtBox[1].y = 53;
    txtBox[1].length = 14;
    txtBox[1].type = 0;
    strcpy(txtBox[1].text, config.wifi_pass);

    display.setCursor(55, 42);
    display.print("PWR:");
    display.setCursor(110, 42);
    display.print("dBm");
    cbBox.isValue = true;
    cbBox.x = 80;
    cbBox.y = 40;
    cbBox.length = 2;
    cbBox.maxItem(20);
    cbBox.char_max = 20;
    cbBox.SetIndex(config.wifi_power);

    display.display();
    encoder0Pos = 0;
    delay(100);
    do
    {
        if (encoder0Pos >= max_sel)
            encoder0Pos = 0;
        if (encoder0Pos < 0)
            encoder0Pos = max_sel - 1;
        if (keyPrev != encoder0Pos)
        {
            keyPrev = encoder0Pos;
            for (i = 0; i < 2; i++)
                txtBox[i].isSelect = false;
            chkBoxWiFi.isSelect = false;
            cbBox.isSelect = false;
            if (encoder0Pos < 2)
                txtBox[encoder0Pos].isSelect = true;
            if (encoder0Pos == 2)
                chkBoxWiFi.isSelect = true;
            if (encoder0Pos == 3)
                cbBox.isSelect = true;
            for (i = 0; i < 2; i++)
                txtBox[i].TextBoxShow();
            chkBoxWiFi.CheckBoxShow();
            cbBox.Show();
        }
        else
        {
            delay(50);
        }
        if (digitalRead(keyPush) == LOW)
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                if ((millis() - currentTime) > 2000)
                {
                    // msgBox("KEY Back");
                    break; // OK Timeout
                }
            };
            if ((millis() - currentTime) < 1500)
            {
                i = encoder0Pos;
                if (encoder0Pos < 2)
                {
                    txtBox[i].TextBox();
                    switch (i)
                    {
                    case 0:
                        strcpy(config.wifi_ssid, txtBox[0].text);
                        break;
                    case 1:
                        strcpy(config.wifi_pass, txtBox[1].text);
                        break;
                    }
                    encoder0Pos = keyPrev + 1;
                }
                else if (encoder0Pos == 2)
                {
                    chkBoxWiFi.Toggle();
                    config.wifi = chkBoxWiFi.Checked;
                    encoder0Pos = keyPrev;
                    chkBoxWiFi.CheckBoxShow();
                }
                else if (encoder0Pos == 3)
                {
                    cbBox.SelectValue(0, 20, 1);
                    config.wifi_power = (unsigned char)cbBox.GetValue();
                    encoder0Pos = keyPrev;
                    cbBox.Show();
                }
                while (digitalRead(keyPush) == LOW)
                    ;
            }
            else
            {
                break;
            }
        }
    } while (1);
    /*display.clearDisplay();
    display.setCursor(30, 4);
    display.print("SAVE & EXIT");
    display.display();*/
    msgBox("KEY EXIT");
    while (digitalRead(keyPush) == LOW)
        ;
    // if (config.wifi_enable)
    // {
    //     WiFi.disconnect(false);
    //     conStatNetwork = CON_WIFI;
    // }
    // else
    // {
    //     WiFi.disconnect(true);
    //     conStatNetwork = CON_WIFI;
    // }
}
void on_stationconfig_selected(MenuItem *p_menu_item)
{
    int max_sel = 7;
    MyTextBox txtBox[3];
    MySymbolBox symBox;
    MyCheckBox chkGPS, chkSMBeacon;
    MyComboBox cbBox;
    String str;
    char ch[10];
    int x, i;
    int keyPrev = -1;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("MY IGATE/STATION");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    // str = String(mycallsign);
    // str.toUpperCase();
    // str.toCharArray(&ch[0],10);
    display.setCursor(0, 18);
    display.print("MyCall:");
    txtBox[0].x = 41;
    txtBox[0].y = 16;
    txtBox[0].length = 7;
    txtBox[0].type = 0;
    str = String(config.aprs_mycall);
    str.toUpperCase();
    str.toCharArray(&ch[0], 10);
    strcpy(txtBox[0].text, ch);
    // TextBoxShow(&ch[0], 44, 16, 7);
    // strcpy(mycallsign, ch);

    // sprintf(ch, "%d", myssid);
    display.setCursor(95, 18);
    display.print("-");
    cbBox.isValue = true;
    cbBox.x = 101;
    cbBox.y = 16;
    cbBox.length = 2;
    cbBox.maxItem(15);
    cbBox.char_max = 15;
    cbBox.SetIndex(config.aprs_ssid);
    // txtBox[1].x = 106;
    // txtBox[1].y = 16;
    // txtBox[1].length = 2;
    // txtBox[1].type = 1;
    // sprintf(txtBox[1].text, "%d", config.myssid);
    // TextBoxShow(&ch[0], 106, 16, 2);
    // TextBoxArrayShow(&txtBox[0]);
    // myssid = atol(ch);

    display.setCursor(0, 30);
    display.print("LAT:");
    // TextBoxShow(&mylat[0], 26, 28, 8);
    txtBox[1].x = 26;
    txtBox[1].y = 28;
    txtBox[1].length = 8;
    txtBox[1].type = 1;
    sprintf(txtBox[1].text, "%.5f", config.gps_lat);

    display.setCursor(0, 42);
    display.print("LON:");
    // TextBoxShow(&mylon[0], 26, 40,9);
    txtBox[2].x = 26;
    txtBox[2].y = 40;
    txtBox[2].length = 9;
    txtBox[2].type = 1;
    sprintf(txtBox[2].text, "%.5f", config.gps_lon);

    chkGPS.Checked = config.mygps;
    chkGPS.isSelect = false;
    chkGPS.x = 0;
    chkGPS.y = 54;
    sprintf(chkGPS.text, "GPS");
    // chkGPS.CheckBoxShow();

    chkSMBeacon.Checked = config.sta_smartbeacon;
    chkSMBeacon.isSelect = false;
    chkSMBeacon.x = 35;
    chkSMBeacon.y = 54;
    sprintf(chkSMBeacon.text, "SmartBCN");

    display.fillRect(98, 31, 30, 11, WHITE);
    display.setTextColor(BLACK);
    display.setCursor(100, 33);
    display.print("ICON");
    display.setTextColor(WHITE);
    symBox.x = 98;
    symBox.y = 43;
    symBox.table = config.mysymbol[0];
    symBox.symbol = config.mysymbol[1];
    symBox.SetIndex(config.mysymbol[1]);

    // txtBox[4].x = 44;
    // txtBox[4].y = 52;
    // txtBox[4].length = 2;
    // strcpy(txtBox[4].text, config.mysymbol);

    display.display();
    encoder0Pos = 0;
    delay(100);
    do
    {
        if (encoder0Pos >= max_sel)
            encoder0Pos = 0;
        if (encoder0Pos < 0)
            encoder0Pos = max_sel - 1;
        if (keyPrev != encoder0Pos)
        {
            keyPrev = encoder0Pos;
            for (i = 0; i < max_sel; i++)
            {
                if (i < 3)
                    txtBox[i].isSelect = false;
                if (i == 3)
                    cbBox.isSelect = false;
                if (i == 6)
                    symBox.isSelect = false;
                if (i == 5)
                    chkSMBeacon.isSelect = false;
                if (i == 4)
                    chkGPS.isSelect = false;
            }
            if (encoder0Pos < 3)
                txtBox[encoder0Pos].isSelect = true;
            if (encoder0Pos == 6)
                symBox.isSelect = true;
            if (encoder0Pos == 5)
                chkSMBeacon.isSelect = true;
            if (encoder0Pos == 4)
                chkGPS.isSelect = true;
            if (encoder0Pos == 3)
                cbBox.isSelect = true;
            for (i = 0; i < 3; i++)
                txtBox[i].TextBoxShow();
            symBox.Show();
            chkGPS.CheckBoxShow();
            chkSMBeacon.CheckBoxShow();
            cbBox.Show();
        }
        else
        {
            delay(50);
        }
        if (digitalRead(keyPush) == LOW)
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                if ((millis() - currentTime) > 2000)
                {
                    break; // OK Timeout
                }
            };
            if ((millis() - currentTime) < 1500)
            {
                i = encoder0Pos;
                if (i == 6)
                {
                    symBox.SelectItem();
                    config.mysymbol[0] = symBox.table;
                    config.mysymbol[1] = symBox.symbol;
                    encoder0Pos = keyPrev;
                }
                else if (i == 4)
                {
                    chkGPS.Toggle();
                    config.mygps = chkGPS.Checked;
                    if (config.mygps == false)
                    {
                        chkSMBeacon.Checked = false;
                        chkSMBeacon.CheckBoxShow();
                    }
                    encoder0Pos = keyPrev;
                    chkGPS.CheckBoxShow();
                }
                else if (i == 5)
                {
                    chkSMBeacon.Toggle();
                    config.sta_smartbeacon = chkSMBeacon.Checked;
                    encoder0Pos = keyPrev;
                    chkSMBeacon.CheckBoxShow();
                }
                else if (i == 3)
                {
                    cbBox.SelectValue(0, 15, 1);
                    config.aprs_ssid = cbBox.GetValue();
                    encoder0Pos = keyPrev;
                    cbBox.Show();
                }
                else
                {
                    txtBox[i].TextBox();
                    switch (i)
                    {
                    case 0:
                        strncpy(config.aprs_mycall, txtBox[i].text, 7);
                        config.aprs_mycall[7] = 0;
                        break;
                    // case 1: config.myssid = atol(txtBox[i].text);
                    //	break;
                    case 1:
                        config.gps_lat = atof(txtBox[i].text); // strcpy(config.mylat, txtBox[i].text);
                        break;
                    case 2:
                        config.gps_lon = atof(txtBox[i].text); // strcpy(config.mylon, txtBox[i].text);
                        break;
                        // case 4: strcpy(config.mysymbol, txtBox[i].text);
                        //	break;
                    }
                    encoder0Pos = keyPrev + 1;
                }
                while (digitalRead(keyPush) == LOW)
                    ;
            }
            else
            {
                break;
            }
        }
    } while (1);
    /*display.clearDisplay();
    display.setCursor(30, 4);
    display.print("SAVE & EXIT");
    display.display();*/
    msgBox("KEY EXIT");
    while (digitalRead(keyPush) == LOW)
        ;
    saveEEPROM();
}
void on_aprsserver_selected(MenuItem *p_menu_item)
{
    int max_sel = 4;
    MyTextBox txtBox[3];
    MyComboBox cbBox;
    String str;
    // char ch[10];
    int x, i;
    int keyPrev = -1;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("APRS SERVER CONFIG");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    display.setCursor(0, 18);
    display.print("SERVER:");
    txtBox[0].x = 0;
    txtBox[0].y = 28;
    txtBox[0].length = 17;
    txtBox[0].type = 0;
    strcpy(txtBox[0].text, config.aprs_host);

    display.setCursor(55, 18);
    display.print("PORT:");
    txtBox[1].x = 85;
    txtBox[1].y = 16;
    txtBox[1].length = 5;
    txtBox[1].type = 1;
    sprintf(txtBox[1].text, "%d", config.aprs_port);
    // strcpy(txtBox[1].text, config.wifi_password);

    display.setCursor(0, 42);
    display.print("Filter:");
    txtBox[2].x = 0;
    txtBox[2].y = 52;
    txtBox[2].length = 17;
    txtBox[2].type = 0;
    strcpy(txtBox[2].text, config.aprs_filter);

    cbBox.isValue = false;
    cbBox.x = 53;
    cbBox.y = 40;
    cbBox.length = 8;
    cbBox.AddItem(0, "CALLSIGN");   // g/HS*/E2*
    cbBox.AddItem(1, "THAI MSG");   // g/HS*/E2*
    cbBox.AddItem(2, "THAI ALL");   // b/HS*/E2*
    cbBox.AddItem(3, "THAI IGATE"); // e/HS*/E2*
    cbBox.AddItem(4, "THAI DIGI");  // d/HS*/E2*
    cbBox.AddItem(5, "NO RECV");    // m/1
    cbBox.maxItem(6);
    // cbBox.char_max = 999;
    cbBox.SetIndex(0);

    display.display();
    encoder0Pos = 0;
    delay(100);
    do
    {
        if (encoder0Pos >= max_sel)
            encoder0Pos = 0;
        if (encoder0Pos < 0)
            encoder0Pos = max_sel - 1;
        if (keyPrev != encoder0Pos)
        {
            keyPrev = encoder0Pos;
            for (i = 0; i < 3; i++)
                txtBox[i].isSelect = false;
            cbBox.isSelect = false;
            if (encoder0Pos < 3)
                txtBox[encoder0Pos].isSelect = true;
            else
                cbBox.isSelect = true;
            for (i = 0; i < 3; i++)
                txtBox[i].TextBoxShow();
            cbBox.Show();
        }
        else
        {
            delay(50);
        }
        if (digitalRead(keyPush) == LOW)
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                if ((millis() - currentTime) > 2000)
                {
                    break; // OK Timeout
                }
            };
            if ((millis() - currentTime) < 1500)
            {
                i = encoder0Pos;
                if (i < 3)
                {
                    txtBox[i].TextBox();
                    switch (i)
                    {
                    case 0:
                        strcpy(config.aprs_host, txtBox[0].text);
                        break;
                    case 1:
                        config.aprs_port = atol(txtBox[1].text);
                        break;
                    case 2:
                        strcpy(config.aprs_filter, txtBox[2].text);
                        break;
                    }
                }
                else
                {
                    cbBox.SelectItem();
                    switch (cbBox.GetIndex())
                    {
                    case 0:
                        strcpy(txtBox[2].text, "b/HS5TQA-9");
                        break;
                    case 1:
                        strcpy(txtBox[2].text, "g/HS*/E2*");
                        break;
                    case 2:
                        strcpy(txtBox[2].text, "b/HS*/E2*");
                        break;
                    case 3:
                        strcpy(txtBox[2].text, "e/HS*/E2*");
                        break;
                    case 4:
                        strcpy(txtBox[2].text, "d/HS*/E2*");
                        break;
                    case 5:
                        strcpy(txtBox[2].text, "m/1");
                        break;
                    }
                    strcpy(config.aprs_filter, txtBox[2].text);
                }
                encoder0Pos = keyPrev + 1;
                while (digitalRead(keyPush) == LOW)
                    ;
            }
            else
            {
                break;
            }
        }
    } while (1);
    /*display.clearDisplay();
    display.setCursor(30, 4);
    display.print("SAVE & EXIT");
    display.display();*/
    msgBox("KEY EXIT");
    while (digitalRead(keyPush) == LOW)
        ;
    saveEEPROM();
    // client.flush();
    // client.clearWriteError();
    // delay(500);
    // client.stop();
    // conStatNetwork = CON_SERVER;
}
void on_tncfunction_selected(MenuItem *p_menu_item)
{
    int max_sel = 9;
    // MyTextBox txtBox[2];
    MyCheckBox chkBox[9];
    String str;
    // char ch[10];
    int x, i;
    int keyPrev = -1;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("nTNC SETTING");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    chkBox[0].Checked = config.tnc;
    chkBox[0].x = 0;
    chkBox[0].y = 18;
    sprintf(chkBox[0].text, "TNC");

    chkBox[1].Checked = config.tnc_digi;
    chkBox[1].x = 35;
    chkBox[1].y = 18;
    sprintf(chkBox[1].text, "DIGI");

    chkBox[2].Checked = config.tnc_tracker;
    chkBox[2].x = 75;
    chkBox[2].y = 18;
    sprintf(chkBox[2].text, "TRACKER");

    chkBox[3].Checked = config.tnc_beacon;
    chkBox[3].x = 0;
    chkBox[3].y = 28;
    sprintf(chkBox[3].text, "BCN");

    chkBox[4].Checked = config.tnc_telemetry;
    chkBox[4].x = 35;
    chkBox[4].y = 28;
    sprintf(chkBox[4].text, "RF_TLM");

    chkBox[5].Checked = config.tnc_compress;
    chkBox[5].x = 0;
    chkBox[5].y = 38;
    sprintf(chkBox[5].text, "COMP");

    chkBox[6].Checked = config.tnc_telemetry;
    chkBox[6].x = 60;
    chkBox[6].y = 38;
    sprintf(chkBox[6].text, "INET_TLM");

    if (config.tnc_telemetry)
    {
        chkBox[4].Checked = false;
    }

    chkBox[7].Checked = config.rf2inet;
    chkBox[7].x = 0;
    chkBox[7].y = 48;
    sprintf(chkBox[7].text, "RF->INET");

    chkBox[8].Checked = config.inet2rf;
    chkBox[8].x = 60;
    chkBox[8].y = 48;
    sprintf(chkBox[8].text, "INET->RF");

    display.display();
    encoder0Pos = 0;
    delay(100);
    do
    {
        if (encoder0Pos >= max_sel)
            encoder0Pos = 0;
        if (encoder0Pos < 0)
            encoder0Pos = max_sel - 1;
        if (keyPrev != encoder0Pos)
        {
            keyPrev = encoder0Pos;
            for (i = 0; i < max_sel; i++)
                chkBox[i].isSelect = false;
            chkBox[encoder0Pos].isSelect = true;
            for (i = 0; i < max_sel; i++)
                chkBox[i].CheckBoxShow();
        }
        else
        {
            delay(50);
        }
        if (digitalRead(keyPush) == LOW)
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                ;
                if ((millis() - currentTime) > 2000)
                    break; // OK Timeout
            };
            if ((millis() - currentTime) < 1500)
            {
                i = encoder0Pos;
                chkBox[i].Toggle();
                switch (i)
                {
                case 0:
                    config.tnc = chkBox[i].Checked;
                    break;
                case 1:
                    config.tnc_digi = chkBox[i].Checked;
                    break;
                case 2:
                    config.tnc_tracker = chkBox[i].Checked;
                    break;
                case 3:
                    config.tnc_beacon = chkBox[i].Checked;
                    break;
                case 4:
                    config.tnc_telemetry = chkBox[i].Checked;
                    chkBox[6].Checked = false;
                    break;
                case 5:
                    config.tnc_compress = chkBox[i].Checked;
                    break;
                case 6:
                    config.tnc_telemetry = chkBox[i].Checked;
                    chkBox[4].Checked = false;
                    break;
                case 7:
                    config.rf2inet = chkBox[i].Checked;
                    break;
                case 8:
                    config.inet2rf = chkBox[i].Checked;
                    break;
                }
                encoder0Pos = keyPrev;
                chkBox[i].CheckBoxShow();
                while (digitalRead(keyPush) == LOW)
                    ;
            }
            else
            {
                break;
            }
        }
    } while (1);
    /*display.clearDisplay();
    display.setCursor(30, 4);
    display.print("SAVE & EXIT");
    display.display();*/
    msgBox("KEY EXIT");
    while (digitalRead(keyPush) == LOW)
        ;
    saveEEPROM();
}

// void on_tnctracker_selected(MenuItem *p_menu_item)
// {
//     int i;
//     MyCheckBox chkBox[3];
//     int max_sel = 8;
//     MySymbolBox symBox[2];
//     MyComboBox cbBox[3];
//     String str;
//     int x;
//     int keyPrev = -1;
//     display.clearDisplay();
//     display.fillRect(0, 0, 128, 16, WHITE);
//     display.setTextColor(BLACK);
//     str = String("TRACKER CONFIG");
//     x = str.length() * 6;
//     display.setCursor(64 - (x / 2), 4);
//     display.print(str);
//     display.setTextColor(WHITE);

//     chkBox[0].Checked = config.tnc_compress;
//     chkBox[0].x = 0;
//     chkBox[0].y = 18;
//     sprintf(chkBox[0].text, "COMP");

//     chkBox[1].Checked = config.tnc_altitude;
//     chkBox[1].x = 40;
//     chkBox[1].y = 18;
//     sprintf(chkBox[1].text, "ALT");

//     chkBox[2].Checked = config.tnc_speed;
//     chkBox[2].x = 75;
//     chkBox[2].y = 18;
//     sprintf(chkBox[2].text, "CSR/SPD");

//     display.setCursor(0, 30);
//     display.print("SPD:");
//     cbBox[0].isValue = true;
//     cbBox[0].x = 25;
//     cbBox[0].y = 28;
//     cbBox[0].length = 3;
//     cbBox[0].char_max = 250;
//     cbBox[0].SetIndex(config.tnc_hspeed);

//     display.setCursor(0, 42);
//     display.print("INV:");
//     cbBox[1].isValue = true;
//     cbBox[1].x = 25;
//     cbBox[1].y = 40;
//     cbBox[1].length = 2;
//     cbBox[1].char_max = 120;
//     cbBox[1].SetIndex(config.tnc_maxinterval);

//     display.setCursor(0, 54);
//     display.print("ANG:");
//     cbBox[2].isValue = true;
//     cbBox[2].x = 25;
//     cbBox[2].y = 52;
//     cbBox[2].length = 2;
//     cbBox[2].char_max = 180;
//     cbBox[2].SetIndex(config.tnc_minangle);

//     display.setCursor(67, 35);
//     display.print("MOV");
//     symBox[0].x = 65;
//     symBox[0].y = 43;
//     symBox[0].table = config.tnc_symmove[0];
//     symBox[0].symbol = config.tnc_symmove[1];
//     symBox[0].SetIndex(config.tnc_symmove[1]);
//     symBox[0].Show();

//     display.setCursor(99, 35);
//     display.print("STP");
//     symBox[1].x = 98;
//     symBox[1].y = 43;
//     symBox[1].table = config.tnc_symstop[0];
//     symBox[1].symbol = config.tnc_symstop[1];
//     symBox[1].SetIndex(config.tnc_symstop[1]);
//     symBox[1].Show();

//     display.display();
//     encoder0Pos = 0;
//     delay(100);
//     do
//     {
//         if (encoder0Pos >= max_sel)
//             encoder0Pos = 0;
//         if (encoder0Pos < 0)
//             encoder0Pos = max_sel - 1;
//         if (keyPrev != encoder0Pos)
//         {
//             keyPrev = encoder0Pos;
//             for (i = 0; i < 3; i++)
//             {
//                 chkBox[i].isSelect = false;
//                 cbBox[i].isSelect = false;
//             }
//             symBox[0].isSelect = false;
//             symBox[1].isSelect = false;

//             if (encoder0Pos < 3)
//                 chkBox[encoder0Pos].isSelect = true;
//             if (encoder0Pos > 2 && encoder0Pos < 6)
//                 cbBox[encoder0Pos - 3].isSelect = true;
//             if (encoder0Pos > 5)
//                 symBox[encoder0Pos - 6].isSelect = true;
//             for (i = 0; i < 3; i++)
//             {
//                 chkBox[i].CheckBoxShow();
//                 cbBox[i].Show();
//             }
//             symBox[0].Show();
//             symBox[1].Show();
//         }
//         else
//         {
//             delay(50);
//         }
//         if (digitalRead(keyPush) == LOW)
//         {
//             currentTime = millis();
//             while (digitalRead(keyPush) == LOW)
//             {
//                 if ((millis() - currentTime) > 2000)
//                     break; // OK Timeout
//             };
//             if ((millis() - currentTime) < 1500)
//             {
//                 i = encoder0Pos;
//                 if (i < 3)
//                 {
//                     chkBox[i].Toggle();
//                     switch (i)
//                     {
//                     case 0:
//                         config.tnc_compress = chkBox[i].Checked;
//                         break;
//                     case 1:
//                         config.tnc_altitude = chkBox[i].Checked;
//                         break;
//                     case 2:
//                         config.tnc_speed = chkBox[i].Checked;
//                         break;
//                     }
//                     encoder0Pos = keyPrev;
//                     chkBox[i].CheckBoxShow();
//                 }
//                 else if (i > 2 && i < 6)
//                 {
//                     i -= 3;
//                     switch (i)
//                     {
//                     case 0:
//                         cbBox[i].SelectValue(10, 200, 1);
//                         config.tnc_hspeed = cbBox[i].GetValue();
//                         break;
//                     case 1:
//                         cbBox[i].SelectValue(5, 60, 1);
//                         config.tnc_maxinterval = cbBox[i].GetValue();
//                         break;
//                     case 2:
//                         cbBox[i].SelectValue(5, 90, 1);
//                         config.tnc_minangle = cbBox[i].GetValue();
//                         break;
//                     }
//                     encoder0Pos = keyPrev;
//                     cbBox[i].Show();
//                 }
//                 else if (encoder0Pos > 5)
//                 {
//                     i -= 6;
//                     symBox[i].SelectItem();
//                     switch (i)
//                     {
//                     case 0:
//                         config.tnc_symmove[0] = symBox[i].table;
//                         config.tnc_symmove[1] = symBox[i].symbol;
//                         break;
//                     case 1:
//                         config.tnc_symstop[0] = symBox[i].table;
//                         config.tnc_symstop[1] = symBox[i].symbol;
//                         break;
//                     }
//                     encoder0Pos = keyPrev;
//                     symBox[i].Show();
//                 }
//                 while (digitalRead(keyPush) == LOW)
//                     ;
//             }
//             else
//             {
//                 break;
//             }
//         }
//     } while (1);
//     /*display.clearDisplay();
//     display.setCursor(30, 4);
//     display.print("SAVE & EXIT");
//     display.display();*/
//     msgBox("KEY EXIT");
//     while (digitalRead(keyPush) == LOW)
//         ;
//     saveEEPROM();
// }

void on_filter_selected(MenuItem *p_menu_item)
{
    int max_sel = 11;
    // MyTextBox txtBox[2];
    MyCheckBox chkBox[11];
    MyComboBox cbBox[2];
    String str;
    // char ch[10];
    int x, i;
    int keyPrev = -1;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("Display CFG");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    chkBox[0].Checked = config.dispDelay;
    chkBox[0].x = 0;
    chkBox[0].y = 16;
    sprintf(chkBox[0].text, "TNC");

    chkBox[1].Checked = config.dispINET;
    chkBox[1].x = 35;
    chkBox[1].y = 16;
    sprintf(chkBox[1].text, "INET");

    chkBox[2].Checked = config.filterStatus;
    chkBox[2].x = 75;
    chkBox[2].y = 16;
    sprintf(chkBox[2].text, "STATUS");

    chkBox[3].Checked = config.filterWeather;
    chkBox[3].x = 0;
    chkBox[3].y = 25;
    sprintf(chkBox[3].text, "WX");

    chkBox[4].Checked = config.filterTelemetry;
    chkBox[4].x = 35;
    chkBox[4].y = 25;
    sprintf(chkBox[4].text, "TLM");

    chkBox[5].Checked = config.filterTracker;
    chkBox[5].x = 75;
    chkBox[5].y = 25;
    sprintf(chkBox[5].text, "TRACKER");

    chkBox[6].Checked = config.filterMessage;
    chkBox[6].x = 0;
    chkBox[6].y = 34;
    sprintf(chkBox[6].text, "MSG");

    chkBox[7].Checked = config.filterMove;
    chkBox[7].x = 35;
    chkBox[7].y = 34;
    sprintf(chkBox[7].text, "MOVE");

    chkBox[8].Checked = config.filterPosition;
    chkBox[8].x = 75;
    chkBox[8].y = 34;
    sprintf(chkBox[8].text, "STATION");

    chkBox[9].Checked = config.h_up;
    chkBox[9].x = 0;
    chkBox[9].y = 43;
    sprintf(chkBox[9].text, "H-UP");

    chkBox[10].Checked = config.tx_status;
    chkBox[10].x = 35;
    chkBox[10].y = 43;
    sprintf(chkBox[10].text, "TXS");

    display.setCursor(0, 54);
    display.print("DLY:");
    // display.setCursor(55, 54);
    // display.print("S");
    cbBox[0].isValue = true;
    cbBox[0].x = 25;
    cbBox[0].y = 52;
    cbBox[0].length = 3;
    cbBox[0].char_max = 999;
    cbBox[0].SetIndex(config.dispDelay);

    display.setCursor(64, 54);
    display.print("DIST<");
    cbBox[1].isValue = true;
    cbBox[1].x = 64 + 30;
    cbBox[1].y = 52;
    cbBox[1].length = 3;
    cbBox[1].char_max = 999;
    cbBox[1].SetIndex(config.filterDistant);

    display.display();
    encoder0Pos = 0;
    delay(100);
    do
    {
        if (encoder0Pos >= 13)
            encoder0Pos = 0;
        if (encoder0Pos < 0)
            encoder0Pos = 12;
        if (keyPrev != encoder0Pos)
        {
            keyPrev = encoder0Pos;
            for (i = 0; i < max_sel; i++)
            {
                chkBox[i].isSelect = false;
            }
            for (i = 0; i < 2; i++)
                cbBox[i].isSelect = false;
            for (i = 0; i < max_sel; i++)
                chkBox[i].isSelect = false;
            if (encoder0Pos < 11)
                chkBox[encoder0Pos].isSelect = true;
            if (encoder0Pos > 10 && encoder0Pos < 13)
                cbBox[encoder0Pos - 11].isSelect = true;
            for (i = 0; i < max_sel; i++)
                chkBox[i].CheckBoxShow();
            for (i = 0; i < 2; i++)
            {
                cbBox[i].Show();
            }
        }
        else
        {
            delay(50);
        }
        if (digitalRead(keyPush) == LOW)
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                if ((millis() - currentTime) > 2000)
                    break; // OK Timeout
            };
            if ((millis() - currentTime) < 1500)
            {
                i = encoder0Pos;
                if (i < 11)
                {
                    chkBox[i].Toggle();
                    switch (i)
                    {
                    case 0:
                        config.dispTNC = chkBox[i].Checked;
                        break;
                    case 1:
                        config.dispINET = chkBox[i].Checked;
                        break;
                    case 2:
                        config.filterStatus = chkBox[i].Checked;
                        break;
                    case 3:
                        config.filterWeather = chkBox[i].Checked;
                        break;
                    case 4:
                        config.filterTelemetry = chkBox[i].Checked;
                        break;
                    case 5:
                        config.filterTracker = chkBox[i].Checked;
                        break;
                    case 6:
                        config.filterMessage = chkBox[i].Checked;
                        break;
                    case 7:
                        config.filterMove = chkBox[i].Checked;
                        break;
                    case 8:
                        config.filterPosition = chkBox[i].Checked;
                        break;
                    case 9:
                        config.h_up = chkBox[i].Checked;
                        break;
                    case 10:
                        config.tx_status = chkBox[i].Checked;
                        break;
                    }
                    encoder0Pos = keyPrev;
                    chkBox[i].CheckBoxShow();
                }
                else if (i > 10 && i < 13)
                {
                    i -= 11;
                    switch (i)
                    {
                    case 0:
                        cbBox[i].SelectValue(1, 600, 1);
                        config.dispDelay = (unsigned int)cbBox[i].GetValue();
                        break;
                    case 1:
                        cbBox[i].SelectValue(0, 999, 1);
                        config.filterDistant = (unsigned int)cbBox[i].GetValue();
                        break;
                    }
                    encoder0Pos = keyPrev;
                    cbBox[i].Show();
                }
                while (digitalRead(keyPush) == LOW)
                    ;
            }
            else
            {
                break;
            }
        }
    } while (1);
    /*display.clearDisplay();
    display.setCursor(30, 4);
    display.print("SAVE & EXIT");
    display.display();*/
    msgBox("KEY EXIT");
    while (digitalRead(keyPush) == LOW)
        ;
    saveEEPROM();
}

void on_tncconfig_selected(MenuItem *p_menu_item)
{
    int max_sel = 6;
    MyTextBox txtBox[5];
    // MySymbolBox symBox;
    // MyCheckBox chkGPS;
    MyComboBox cbBox;
    MyCheckBox chkEn;
    String str;
    char ch[10];
    int x, i;
    int keyPrev = -1;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("nTNC CONFIGURATION");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    display.setCursor(0, 18);
    display.print("MyCall:");
    txtBox[0].x = 41;
    txtBox[0].y = 16;
    txtBox[0].length = 7;
    txtBox[0].type = 0;
    str = String(config.aprs_mycall);
    str.toUpperCase();
    str.toCharArray(&ch[0], 10);
    strcpy(txtBox[0].text, ch);

    // sprintf(ch, "%d", myssid);
    display.setCursor(95, 18);
    display.print("-");
    cbBox.isValue = true;
    cbBox.x = 101;
    cbBox.y = 16;
    cbBox.length = 2;
    cbBox.maxItem(15);
    cbBox.char_max = 15;
    cbBox.SetIndex(config.aprs_ssid);
    // txtBox[1].x = 106;
    // txtBox[1].y = 16;
    // txtBox[1].length = 2;
    // txtBox[1].type = 1;
    // sprintf(txtBox[1].text, "%d", config.aprs_ssid);

    chkEn.Checked = config.tnc;
    chkEn.x = 98;
    chkEn.y = 29;
    sprintf(chkEn.text, "TNC");

    display.setCursor(0, 30);
    display.print("ITEM:");
    txtBox[2].x = 30;
    txtBox[2].y = 28;
    txtBox[2].length = 9;
    strcpy(txtBox[2].text, config.tnc_item);

    display.setCursor(0, 42);
    display.print("PTH:");
    txtBox[3].x = 25;
    txtBox[3].y = 40;
    txtBox[3].length = 14;
    strcpy(txtBox[3].text, config.tnc_path);

    display.setCursor(0, 54);
    display.print("CMN:");
    txtBox[4].x = 25;
    txtBox[4].y = 52;
    txtBox[4].length = 14;
    strcpy(txtBox[4].text, config.tnc_comment);

    display.display();
    encoder0Pos = 0;
    delay(100);
    do
    {
        if (encoder0Pos >= max_sel)
            encoder0Pos = 0;
        if (encoder0Pos < 0)
            encoder0Pos = max_sel - 1;
        if (keyPrev != encoder0Pos)
        {
            keyPrev = encoder0Pos;
            for (i = 0; i < max_sel; i++)
            {
                if (i == 1)
                    cbBox.isSelect = false;
                else if (i == 5)
                    chkEn.isSelect = false;
                else
                    txtBox[i].isSelect = false;
            }
            if (encoder0Pos == 1)
                cbBox.isSelect = true;
            else if (encoder0Pos == 5)
                chkEn.isSelect = true;
            else
                txtBox[encoder0Pos].isSelect = true;
            for (i = 0; i < max_sel; i++)
            {
                if (i == 1)
                    cbBox.Show();
                else if (i == 5)
                    chkEn.CheckBoxShow();
                else
                    txtBox[i].TextBoxShow();
            }
        }
        else
        {
            delay(50);
        }
        if (digitalRead(keyPush) == LOW)
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                if ((millis() - currentTime) > 2000)
                    break; // OK Timeout
            };
            if ((millis() - currentTime) < 1500)
            {
                i = encoder0Pos;
                if (i == 1)
                {
                    cbBox.SelectValue(0, 15, 1);
                    config.aprs_ssid = cbBox.GetValue();
                    encoder0Pos = keyPrev;
                }
                else if (i == 5)
                {
                    chkEn.Toggle();
                    config.tnc = chkEn.Checked;
                    encoder0Pos = keyPrev;
                    chkEn.CheckBoxShow();
                }
                else
                {
                    txtBox[i].TextBox();
                    switch (i)
                    {
                    case 0:
                        strncpy(config.aprs_mycall, txtBox[i].text, 7);
                        config.aprs_mycall[7] = 0;
                        break;
                    // case 1: config.aprs_ssid = atol(txtBox[i].text);
                    //	break;
                    case 2:
                        strcpy(config.tnc_item, txtBox[i].text);
                        break;
                    case 3:
                        strcpy(config.tnc_path, txtBox[i].text);
                        break;
                    case 4:
                        strcpy(config.tnc_comment, txtBox[i].text);
                        break;
                    }
                    encoder0Pos = keyPrev + 1;
                }
                while (digitalRead(keyPush) == LOW)
                    ;
            }
            else
            {
                break;
            }
        }
    } while (1);
    // display.clearDisplay();
    // display.setCursor(30, 4);
    // display.print("SAVE & EXIT");
    // display.display();
    msgBox("KEY EXIT");
    while (digitalRead(keyPush) == LOW)
        ;
}

void on_rfconfig_selected(MenuItem *p_menu_item)
{
    MyTextBox txtBox;
    MyCheckBox chkBoxWiFi;
    MyComboBox cbBox[2];
    String str;
    // char ch[10];
    int x, i;
    int max_sel = 4;
    int keyPrev = -1;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("RF Config");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    chkBoxWiFi.Checked = config.tnc_rfmodule;
    chkBoxWiFi.x = 0;
    chkBoxWiFi.y = 18;
    sprintf(chkBoxWiFi.text, "RF_ENABLE");

    display.setCursor(0, 30);
    display.print("FREQ:");
    display.setCursor(100, 30);
    display.print("MHz");
    txtBox.x = 35;
    txtBox.y = 28;
    txtBox.length = 8;
    txtBox.type = 1;
    // strcpy(txtBox.text, config.freq_tx);
    sprintf(txtBox.text, "%.4f", config.freq_tx);

    display.setCursor(0, 42);
    display.print("SEQ:");
    cbBox[0].isValue = true;
    cbBox[0].x = 25;
    cbBox[0].y = 40;
    cbBox[0].length = 1;
    cbBox[0].maxItem(8);
    cbBox[0].char_max = 8;
    cbBox[0].SetIndex(config.sql_level);

    display.setCursor(0, 54);
    display.print("PWR:");
    cbBox[1].isValue = false;
    cbBox[1].x = 25;
    cbBox[1].y = 52;
    cbBox[1].length = 3;
    cbBox[1].AddItem(0, "LOW"); // g/HS*/E2*
    cbBox[1].AddItem(1, "HI");  // g/HS*/E2*
    cbBox[1].maxItem(2);
    // cbBox.char_max = 999;
    cbBox[1].SetIndex(config.rf_power);

    display.display();
    encoder0Pos = 0;
    delay(100);
    do
    {
        if (encoder0Pos >= max_sel)
            encoder0Pos = 0;
        if (encoder0Pos < 0)
            encoder0Pos = max_sel - 1;
        if (keyPrev != encoder0Pos)
        {
            keyPrev = encoder0Pos;
            for (i = 0; i < 2; i++)
                cbBox[i].isSelect = false;
            chkBoxWiFi.isSelect = false;
            txtBox.isSelect = false;
            if (encoder0Pos < 2)
                cbBox[encoder0Pos].isSelect = true;
            if (encoder0Pos == 2)
                chkBoxWiFi.isSelect = true;
            if (encoder0Pos == 3)
                txtBox.isSelect = true;
            for (i = 0; i < 2; i++)
                cbBox[i].Show();
            chkBoxWiFi.CheckBoxShow();
            txtBox.TextBoxShow();
        }
        else
        {
            delay(50);
        }
        if (digitalRead(keyPush) == LOW)
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                delay(10);
                if ((millis() - currentTime) > 2000)
                {
                    // msgBox("KEY Back");
                    break; // OK Timeout
                }
            };
            if ((millis() - currentTime) < 1500)
            {
                i = encoder0Pos;
                if (encoder0Pos == 0)
                {
                    cbBox[0].SelectValue(0, 8, 1);
                    config.sql_level = cbBox[0].GetValue();
                    encoder0Pos = keyPrev + 1;
                    cbBox[1].Show();
                }
                else if (encoder0Pos == 1)
                {
                    // cbBox[1].SelectValue(0, 1, 1);
                    cbBox[1].SelectItem();
                    config.rf_power = cbBox[1].GetIndex();
                    encoder0Pos = keyPrev + 1;
                    cbBox[1].Show();
                }
                else if (encoder0Pos == 2)
                {
                    chkBoxWiFi.Toggle();
                    config.tnc_rfmodule = chkBoxWiFi.Checked;
                    encoder0Pos = keyPrev;
                    chkBoxWiFi.CheckBoxShow();
                }
                else if (encoder0Pos == 3)
                {
                    txtBox.TextBox();
                    // strcpy(config.freq_tx, txtBox.text);
                    config.freq_tx = atof(txtBox.text);
                    encoder0Pos = keyPrev;
                    txtBox.TextBoxShow();
                }
                while (digitalRead(keyPush) == LOW)
                    delay(10);
            }
            else
            {
                break;
            }
        }
        delay(10);
    } while (1);
    /*display.clearDisplay();
    display.setCursor(30, 4);
    display.print("SAVE & EXIT");
    display.display();*/
    msgBox("KEY EXIT");
    while (digitalRead(keyPush) == LOW)
        delay(10);
}

void on_display_selected(MenuItem *p_menu_item)
{
    // MyTextBox txtBox;
    MyCheckBox chkBoxWiFi;
    MyComboBox cbDim, cbContrast, cbStartup;
    String str;
    // char ch[10];
    int x, i;
    int max_sel = 4;
    int keyPrev = -1;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("Display Config");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    chkBoxWiFi.Checked = config.tnc_rfmodule;
    chkBoxWiFi.x = 0;
    chkBoxWiFi.y = 18;
    sprintf(chkBoxWiFi.text, "TITLE");

    display.setCursor(0, 30);
    display.print("DIM:");
    cbDim.isValue = false;
    cbDim.x = 25;
    cbDim.y = 28;
    cbDim.length = 9;
    cbDim.maxItem(5);
    cbDim.AddItem(0, "HI");
    cbDim.AddItem(1, "LOW");
    cbDim.AddItem(2, "AUTO DIM");
    cbDim.AddItem(3, "DAY/NIGHT");
    cbDim.AddItem(4, "CONTRAST");

    // cbBox.char_max = 999;
    cbDim.SetIndex(config.dim);

    display.setCursor(0, 42);
    display.print("CONTRAST:");
    cbContrast.isValue = true;
    cbContrast.x = 55;
    cbContrast.y = 40;
    cbContrast.length = 3;
    cbContrast.maxItem(255);
    cbContrast.char_max = 255;
    cbContrast.SetIndex(config.contrast);

    display.setCursor(0, 54);
    display.print("FDP:");
    cbStartup.isValue = false;
    cbStartup.x = 25;
    cbStartup.y = 52;
    cbStartup.length = 10;
    cbStartup.maxItem(6);
    cbStartup.AddItem(0, "STATUS");
    cbStartup.AddItem(1, "LAST STA");
    cbStartup.AddItem(2, "TOP PKG");
    cbStartup.AddItem(3, "SYS INFO");
    cbStartup.AddItem(4, "GPS INFO");
    cbStartup.AddItem(5, "CST/SPD");
    if (config.startup > 5)
        config.startup = 5;
    cbStartup.SetIndex(config.startup);

    display.display();
    encoder0Pos = 0;
    delay(100);
    do
    {
        if (encoder0Pos >= max_sel)
            encoder0Pos = 0;
        if (encoder0Pos < 0)
            encoder0Pos = max_sel - 1;
        if (keyPrev != encoder0Pos)
        {
            keyPrev = encoder0Pos;
            cbDim.isSelect = false;
            cbContrast.isSelect = false;
            cbStartup.isSelect = false;
            chkBoxWiFi.isSelect = false;
            if (encoder0Pos == 1)
                cbDim.isSelect = true;
            if (encoder0Pos == 2)
                cbContrast.isSelect = true;
            if (encoder0Pos == 3)
                cbStartup.isSelect = true;
            if (encoder0Pos == 0)
                chkBoxWiFi.isSelect = true;
            cbDim.Show();
            cbContrast.Show();
            chkBoxWiFi.CheckBoxShow();
            cbStartup.Show();
        }
        else
        {
            delay(50);
        }
        if (digitalRead(keyPush) == LOW)
        {
            currentTime = millis();
            while (digitalRead(keyPush) == LOW)
            {
                delay(10);
                if ((millis() - currentTime) > 2000)
                {
                    // msgBox("KEY Back");
                    break; // OK Timeout
                }
            };
            if ((millis() - currentTime) < 1500)
            {
                i = encoder0Pos;
                if (encoder0Pos == 0)
                {
                    chkBoxWiFi.Toggle();
                    config.tnc_rfmodule = chkBoxWiFi.Checked;
                    encoder0Pos = keyPrev;
                    chkBoxWiFi.CheckBoxShow();
                }
                else if (encoder0Pos == 1)
                {
                    cbDim.SelectItem();
                    config.dim = cbDim.GetIndex();
                    if (config.dim == 1)
                    {
                        display.dim(true);
                    }
                    else if (config.dim == 4)
                    {
                        // display.dim(true);
                        display.ssd1306_command(SSD1306_SETCONTRAST);
                        display.ssd1306_command(config.contrast);
                    }
                    else
                    {
                        display.dim(false);
                    }
                    cbDim.Show();
                }
                else if (encoder0Pos == 2)
                {
                    cbContrast.SelectValue(0, 200, 1);
                    config.contrast = cbContrast.GetValue();
                    // display.ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
                    // display.ssd1306_command(config.contrast);
                    // display.ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
                    // display.ssd1306_command(config.contrast);
                    display.ssd1306_command(SSD1306_SETCONTRAST);
                    display.ssd1306_command(config.contrast);
                    cbContrast.Show();
                }
                else if (encoder0Pos == 3)
                {
                    cbStartup.SelectItem();
                    config.startup = cbStartup.GetIndex();
                    cbStartup.Show();
                }
                encoder0Pos = keyPrev;
                while (digitalRead(keyPush) == LOW)
                    delay(10);
            }
            else
            {
                break;
            }
        }
        delay(10);
    } while (1);
    /*display.clearDisplay();
    display.setCursor(30, 4);
    display.print("SAVE & EXIT");
    display.display();*/
    msgBox("KEY EXIT");
    while (digitalRead(keyPush) == LOW)
        delay(10);
}

void on_update_selected(MenuItem *p_menu_item)
{
    String str;
    char cstr[300];
    int x;

    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("OTA UPDATE");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    display.setCursor(0, 18);
    display.print("Current V");
    display.printf("%s%c\n", VERSION, VERSION_BUILD);

    display.println("Wait Download/Update");
    display.display();
    // delay(1000);
    delay(10);

    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.disconnect(false);
        delay(500);
        WiFi.mode(WIFI_STA);
        // WiFi.setTxPower(20);
        WiFi.begin(config.wifi_ssid, config.wifi_pass);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(100);
        }
    }

    // wait for WiFi connection
    if ((WiFi.status() == WL_CONNECTED))
    {
        t_httpUpdate_return ret = ESPhttpUpdate.update(String("http://www.dprns.com/ESP32/DRHotspot.bin"), String(VERSION));

        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            sprintf(cstr, "UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            sprintf(cstr, "UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            sprintf(cstr, "UPDATE_OK");
            break;
        }

        SerialLOG.println(cstr);
        display.println(cstr);
        display.print("New Current V");
        display.printf("%s%c\n", VERSION, VERSION_BUILD);
        display.display();
        delay(2000);
    }
    else
    {
        SerialLOG.println("UPDATE FAIL!");
        SerialLOG.println("WiFi Disconnect");
        display.println("UPDATE FAIL!");
        display.println("WiFi Disconnect");
        display.display();
        // delay(1000);
    }
    while (digitalRead(keyPush) == HIGH)
        delay(10);
    WiFi.disconnect(true);
    // display.print("Update Successfully");
    // display.println("To AUTO RESTART");
    // display.display();
    // delay(2000);
    ESP.restart();
}

void on_infomation_selected(MenuItem *p_menu_item)
{
    String str;
    char cstr[50];
    int x;

    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("INFOMATION");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    display.setCursor(0, 18);
    display.print("Firmware V");
    display.printf("%s%c\n", VERSION, VERSION_BUILD);
    display.println("Developer: HS5TQA");
    display.display();
    while (digitalRead(keyPush) == HIGH)
        delay(10);
}

void on_save_selected(MenuItem *p_menu_item)
{
    saveEEPROM();

    display.clearDisplay();
    display.setCursor(52, 4);
    display.print("SAVE");
    display.setCursor(0, 18);
    display.print("Save All Configure\n to EEPROM");
    display.display();
    delay(1000);
    while (digitalRead(keyPush) == LOW)
        delay(10);
}

void on_load_selected(MenuItem *p_menu_item)
{
    uint8_t *ptr;
    int i;
    int addr = 1;

    ptr = (byte *)&config;
    EEPROM.readBytes(1, ptr, sizeof(Configuration));
    uint8_t chkSum = checkSum(ptr, sizeof(Configuration));
    log_d("EEPROM Check %0Xh=%0Xh(%dByte)\n", EEPROM.read(0), chkSum, sizeof(Configuration));

    display.clearDisplay();
    display.setCursor(52, 4);
    display.print("LOAD");
    display.setCursor(0, 18);
    if (EEPROM.read(0) != chkSum)
    {
        display.print("Load Configuration OK!");
        log_d("Config EEPROM Error!");
        // defaultConfig();
    }
    else
    {
        display.print("Load Configuration Fail!");
    }
    display.display();
    delay(1000);
    while (digitalRead(keyPush) == LOW)
        delay(10);
}

void on_factory_selected(MenuItem *p_menu_item)
{
    defaultConfig();
    display.clearDisplay();
    display.setCursor(52, 4);
    display.print("FACTORY");
    display.setCursor(0, 18);
    display.print("Reset configuration to Factory");
    display.display();
    delay(1000);
    while (digitalRead(keyPush) == LOW)
        delay(10);
}

void on_reboot_selected(MenuItem *p_menu_item)
{
    display.clearDisplay();
    display.setCursor(52, 4);
    display.print("REBOOT");
    display.setCursor(0, 18);
    display.print("SYSTEM REBOOT");
    display.display();
    delay(1000);
    while (digitalRead(keyPush) == LOW)
        delay(10);
    WiFi.disconnect(true);
    ESP.restart();
}

void on_dashboard_selected(MenuItem *p_menu_item)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        conStatNetwork = CON_SERVER;
        topBar(WiFi.RSSI());
    }
    else
    {
        conStatNetwork = CON_WIFI;
    }
    conStat = CON_NORMAL;
    display.clearDisplay();
    display.display();
}

void on_wifistatus_selected(MenuItem *p_menu_item)
{
    String str;
    // char ch[10];
    int x;
    // int keyPrev = -1;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("WIFI STATUS");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);

    // display.setTextSize(1);
    display.setCursor(0, 16);
    display.print("SSID: ");
    display.print(WiFi.SSID());
    display.setCursor(0, 24);
    display.print("RSSI: ");
    display.print(WiFi.RSSI());
    display.println(" dBm");
    // display.setCursor(20, 32);
    display.print("MAC ");
    display.println(WiFi.macAddress());
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.print("GW: ");
    display.print(WiFi.gatewayIP());
    display.display();
    while (digitalRead(keyPush) == HIGH)
        delay(10);
}
// void on_back_selected(MenuItem *p_menu_item);

void on_tncmonitor_selected(MenuItem *p_menu_item)
{
    String str;
    // char cstr[50];
    int x;

    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("nTNC Monitor");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);
    display.display();

    while (digitalRead(keyPush) == HIGH)
    {
        delay(10);
        // if (Serial.available() > 0)
        // {
        //     String tnc2 = Serial.readStringUntil('\n');
        //     display.fillRect(0, 16, 128, 48, BLACK);
        //     display.setCursor(1, 18);
        //     display.print(tnc2);
        //     display.display();
        //     delay(2000);
        //     display.fillRect(0, 16, 128, 48, BLACK);
        //     display.display();
        // }
    }
}

void on_txbeacon_selected(MenuItem *p_menu_item)
{
    String str;
    int x;
    String rawTNC = myBeacon(String(",WIDE1-1"));
    // sprintf(cstr, "=%s%c%s%c%s%s\r\n", config.mylat, config.mysymbol[0], config.mylon, config.mysymbol[1], config.myphg, config.mycomment);
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("nTNC TX Beacon");
    x = str.length() * 6;
    // tncTxEnable = false;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);
    display.fillRect(0, 16, 128, 48, BLACK);
    display.setCursor(1, 25);
    display.print(rawTNC);
    display.display();
    // SerialTNC.print("\r\n");
    // SerialTNC.println("}" + rawTNC);
    delay(2000);
    // tncTxEnable = true;
}

void on_txstatus_selected(MenuItem *p_menu_item)
{
    String str;
    char cstr[300];
    int x;
    sprintf(cstr, ">WiFi IGate V%s%c\r\n", VERSION, VERSION_BUILD);
    // SerialTNC.flush();
    // tncTxEnable = false;
    display.clearDisplay();
    display.fillRect(0, 0, 128, 16, WHITE);
    display.setTextColor(BLACK);
    str = String("nTNC TX RAW");
    x = str.length() * 6;
    display.setCursor(64 - (x / 2), 4);
    display.print(str);
    display.setTextColor(WHITE);
    display.fillRect(0, 16, 128, 48, BLACK);
    display.setCursor(1, 25);
    display.print(cstr);
    display.display();
}

byte htod(char *val, int str, int stp)
{
    char cstr[3];
    byte ret;
    cstr[0] = val[str];
    cstr[1] = val[stp];
    cstr[2] = 0;
    ret = (byte)strtol(cstr, 0, 16);
    return ret;
}

// Menu variables
MenuSystem ms(my_renderer);

Menu mnuAbout("ABOUT");
MenuItem mnuAbout_mi1("OTA Update", &on_update_selected);
MenuItem mnuAbout_mi2("WiFi Status", &on_wifistatus_selected);
MenuItem mnuAbout_mi3("Infomations", &on_infomation_selected);
MenuItem mnuAbout_mi4("Dash Board", &on_dashboard_selected);

Menu mnuConfig("Save/Load");
MenuItem mnuConfig_mi1("Save Config", &on_save_selected);
MenuItem mnuConfig_mi2("Load Config", &on_load_selected);
MenuItem mnuConfig_mi3("Factory Reset", &on_factory_selected);

Menu mnu1("WiFi/BT/RF");
MenuItem mnu1_mi1("WiFi AP", &on_wifi_selected);
MenuItem mnu1_mi2("WiFi Client", &on_wifi_selected);
MenuItem mnu1_mi3("Blue Tooth", &on_wifi_selected);
MenuItem mnu1_mi4("RF Module", &on_rfconfig_selected);

Menu mnu2("APRS IGATE");
MenuItem mnu2_mi1("APRS-IS", &on_aprsserver_selected);
MenuItem mnu2_mi2("Position", &on_stationconfig_selected);
MenuItem mnu2_mi3("Filter", &on_filter_selected);
MenuItem mnu2_mi4("Beacon", &on_stationbeacon_selected);

Menu mnu3("APRS TRACKER");
MenuItem mnu3_mi1("Position", &on_stationconfig_selected);
MenuItem mnu3_mi2("Function", &on_tncfunction_selected);
MenuItem mnu3_mi3("Filter", &on_filter_selected);
MenuItem mnu3_mi4("SmartBeacon", &on_stationbeacon_selected);

Menu mnu4("DIGI REPEATER");
MenuItem mnu4_mi1("Position", &on_stationconfig_selected);
MenuItem mnu4_mi2("Function", &on_tncfunction_selected);
MenuItem mnu4_mi3("Filter", &on_filter_selected);
MenuItem mnu4_mi4("SmartBeacon", &on_stationbeacon_selected);

Menu mnu5("SYSTEM");
MenuItem mnu5_mi1("Save/Load", &on_wifi_selected);
MenuItem mnu5_mi2("OLED Display", &on_display_selected);
MenuItem mnu5_mi3("Monitor", &on_filter_selected);

// Menu mu2("CONFIGURATION");
// MenuItem mu2_mi1("WiFi Config", &on_wifi_selected);
// MenuItem mu2_mi2("Station Fix", &on_stationconfig_selected);
// MenuItem mu2_mi3("Smart Beacon", &on_stationbeacon_selected);
// MenuItem mu2_mi4("APRS Server", &on_aprsserver_selected);
// MenuItem mu2_mi5("Filter Display", &on_filter_selected);

// Menu mu6("TNC SETTING");
// MenuItem mu6_mi0("TNC Config", &on_tncconfig_selected);
// MenuItem mu6_mi1("TNC Function", &on_tncfunction_selected);
// MenuItem mu6_mi2("TNC RF Module", &on_rfconfig_selected);
// // MenuItem mu6_mi2("TNC Tracker", &on_tnctracker_selected);
// // MenuItem mu6_mi3("TNC RF Module", &on_rfconfig_selected);
// //  MenuItem mu2_mi5("<--", &on_back_selected);
// Menu mu3("ABOUT");
// MenuItem mu3_mi1("OTA Update", &on_update_selected);
// MenuItem mu3_mi2("WiFi Status", &on_wifistatus_selected);
// MenuItem mu3_mi3("Infomations", &on_infomation_selected);
// MenuItem mu3_mi4("Dash Board", &on_dashboard_selected);
// // MenuItem mu3_mi4("<--", &on_back_selected);
// Menu mu4("SYSTEM");
// MenuItem mu4_mi1("Save Config", &on_save_selected);
// MenuItem mu4_mi2("Load Config", &on_load_selected);
// MenuItem mu4_mi3("Factory Reset", &on_factory_selected);
// MenuItem mu4_mi4("Display Config", &on_display_selected);
// //MenuItem mu4_mi5("Reboot", &on_reboot_selected);
// //MenuItem mu4_mi5()"Reboot", &on_reboot_selected);

// // MenuItem mu4_mi5("Dash Board", &on_system5_selected);
// // MenuItem mu4_mi6("<--", &on_back_selected);
// Menu mu5("TNC CONTROL");
// MenuItem mu5_mi1("nTNC Monitor", &on_tncmonitor_selected);
// MenuItem mu5_mi2("nTNC TX myBeacon", &on_txbeacon_selected);
// MenuItem mu5_mi3("nTNC TX Status", &on_txstatus_selected);

void on_back_selected(MenuItem *p_menu_item)
{
    line = 15;
    ms.back();
    ms.display();
}
// DigoleSerialDisp mydisp = DigoleSerialDisp(8, 9, 10);
// qMenuSystem menu = qMenuSystem(mydisp);

void statisticsDisp()
{

    // uint8_twifi = 0, i;
    int x;
    String str;
    display.fillRect(0, 16, 128, 10, WHITE);
    display.drawLine(0, 16, 0, 63, WHITE);
    display.drawLine(127, 16, 127, 63, WHITE);
    display.drawLine(0, 63, 127, 63, WHITE);
    display.fillRect(1, 25, 126, 38, BLACK);
    display.setTextColor(BLACK);
    display.setCursor(30, 17);
    display.print("STATISTICS");
    display.setCursor(108, 17);
    display.print("1/5");
    display.setTextColor(WHITE);

    // display.setCursor(3, 26);
    // display.print("ALL DATA");
    // str = String(status.allCount, DEC);
    // x = str.length() * 6;
    // display.setCursor(126 - x, 26);
    // display.print(str);

    // display.setCursor(3, 35);
    // display.print("RF2INET");
    // str = String(status.rf2inet, DEC);
    // x = str.length() * 6;
    // display.setCursor(126 - x, 35);
    // display.print(str);

    // display.setCursor(3, 44);
    // display.print("INET2RF");
    // str = String(status.inet2rf, DEC);
    // x = str.length() * 6;
    // display.setCursor(126 - x, 44);
    // display.print(str);

    // display.setCursor(3, 53);
    // display.print("ERROR/DROP");
    // str = String(status.errorCount + status.dropCount, DEC);
    // x = str.length() * 6;
    // display.setCursor(126 - x, 53);
    // display.print(str);

    display.display();
}

void pkgLastDisp()
{

    uint8_t k = 0;
    int i;
    // char list[4];
    int x, y;
    String str;
    // String times;
    // pkgListType *ptr[100];

    display.fillRect(0, 16, 128, 10, WHITE);
    display.drawLine(0, 16, 0, 63, WHITE);
    display.drawLine(127, 16, 127, 63, WHITE);
    display.drawLine(0, 63, 127, 63, WHITE);
    display.fillRect(1, 25, 126, 38, BLACK);
    display.setTextColor(BLACK);
    display.setCursor(27, 17);
    display.print("LAST STATIONS");
    display.setCursor(108, 17);
    display.print("2/5");
    display.setTextColor(WHITE);

    sort(pkgList, PKGLISTSIZE);
    k = 0;
    for (i = 0; i < PKGLISTSIZE; i++)
    {
        if (pkgList[i].time > 0)
        {
            y = 26 + (k * 9);
            // display.drawBitmap(3, y, &SYMBOL[0][0], 11, 6, WHITE);
            display.fillRoundRect(2, y, 7, 8, 2, WHITE);
            display.setCursor(3, y);
            pkgList[i].calsign[10] = 0;
            display.setTextColor(BLACK);
            switch (pkgList[i].type)
            {
            case PKG_OBJECT:
                display.print("O");
                break;
            case PKG_ITEM:
                display.print("I");
                break;
            case PKG_MESSAGE:
                display.print("M");
                break;
            case PKG_WX:
                display.print("W");
                break;
            case PKG_TELEMETRY:
                display.print("T");
                break;
            case PKG_QUERY:
                display.print("Q");
                break;
            case PKG_STATUS:
                display.print("S");
                break;
            default:
                display.print("*");
                break;
            }
            display.setTextColor(WHITE);
            display.setCursor(10, y);
            display.print(pkgList[i].calsign);
            display.setCursor(126 - 48, y);
            display.printf("%02d:%02d:%02d", hour(pkgList[i].time), minute(pkgList[i].time), second(pkgList[i].time));

            // str = String(hour(pkgList[i].time),DEC) + ":" + String(minute(pkgList[i].time), DEC) + ":" + String(second(pkgList[i].time), DEC);
            ////str = String(pkgList[pkgLast_array[i]].time, DEC);
            // x = str.length() * 6;
            // display.setCursor(126 - x, y);
            // display.print(str);
            k++;
            if (k >= 4)
                break;
        }
    }
    display.display();
}

void pkgCountDisp()
{

    // uint8_twifi = 0, k = 0, l;
    uint k = 0;
    int i;
    // char list[4];
    int x, y;
    String str;
    // String times;
    // pkgListType *ptr[100];

    display.fillRect(0, 16, 128, 10, WHITE);
    display.drawLine(0, 16, 0, 63, WHITE);
    display.drawLine(127, 16, 127, 63, WHITE);
    display.drawLine(0, 63, 127, 63, WHITE);
    display.fillRect(1, 25, 126, 38, BLACK);
    display.setTextColor(BLACK);
    display.setCursor(30, 17);
    display.print("TOP PACKAGE");
    display.setCursor(108, 17);
    display.print("3/5");
    display.setTextColor(WHITE);

    sortPkgDesc(pkgList, PKGLISTSIZE);
    k = 0;
    for (i = 0; i < PKGLISTSIZE; i++)
    {
        if (pkgList[i].time > 0)
        {
            y = 26 + (k * 9);
            // display.drawBitmapV(2, y-1, &SYMBOL[pkgList[i].symbol][0], 11, 8, WHITE);
            pkgList[i].calsign[10] = 0;
            display.fillRoundRect(2, y, 7, 8, 2, WHITE);
            display.setCursor(3, y);
            pkgList[i].calsign[10] = 0;
            display.setTextColor(BLACK);
            switch (pkgList[i].type)
            {
            case PKG_OBJECT:
                display.print("O");
                break;
            case PKG_ITEM:
                display.print("I");
                break;
            case PKG_MESSAGE:
                display.print("M");
                break;
            case PKG_WX:
                display.print("W");
                break;
            case PKG_TELEMETRY:
                display.print("T");
                break;
            case PKG_QUERY:
                display.print("Q");
                break;
            case PKG_STATUS:
                display.print("S");
                break;
            default:
                display.print("*");
                break;
            }
            display.setTextColor(WHITE);
            display.setCursor(10, y);
            display.print(pkgList[i].calsign);
            str = String(pkgList[i].pkg, DEC);
            x = str.length() * 6;
            display.setCursor(126 - x, y);
            display.print(str);
            k++;
            if (k >= 4)
                break;
        }
    }
    display.display();
}

void systemDisp()
{

    // uint8_twifi = 0, k = 0, l;
    // char i;
    // char list[4];
    int x;
    String str;
    time_t upTime = now(); // - startTime;
    // String times;
    // pkgListType *ptr[100];

    display.fillRect(0, 16, 128, 10, WHITE);
    display.drawLine(0, 16, 0, 63, WHITE);
    display.drawLine(127, 16, 127, 63, WHITE);
    display.drawLine(0, 63, 127, 63, WHITE);
    display.fillRect(1, 25, 126, 38, BLACK);
    display.setTextColor(BLACK);
    display.setCursor(30, 17);
    display.print("SYSTEM INFO");
    display.setCursor(108, 17);
    display.print("4/5");
    display.setTextColor(WHITE);

    display.setCursor(3, 26);
    display.print("HMEM:");
    str = String(ESP.getFreeHeap(), DEC) + "Byte";
    x = str.length() * 6;
    display.setCursor(126 - x, 26);
    display.print(str);

    display.setCursor(3, 35);
    display.print("UPTIME:");
    str = String(day(upTime) - 1, DEC) + "D " + String(hour(upTime), DEC) + ":" + String(minute(upTime), DEC) + ":" + String(second(upTime), DEC);
    x = str.length() * 6;
    display.setCursor(126 - x, 35);
    display.print(str);

    display.setCursor(3, 44);
    display.print("WIFI:");
    // str = String(str_status[WiFi.status()]);
    str = String(WiFi.status());
    x = str.length() * 6;
    display.setCursor(126 - x, 44);
    display.print(str);

    display.setCursor(3, 53);
    display.print("VERSION:");
    str = String(VERSION) + String(VERSION_BUILD);
    x = str.length() * 6;
    display.setCursor(126 - x, 53);
    display.print(str);

    display.display();
}

void gpsDisp()
{
    //	compass_label(25, 37, 15, 0.0F, WHITE);
    // compass_arrow(25, 37, 12, dtmp, WHITE);
    // uint8_twifi = 0, k = 0, l;
    // char i;
    // char list[4];
    int x;
    String str;
    // time_t upTime = now() - startTime;
    // String times;
    // pkgListType *ptr[100];

    if (gps_mode == 0)
    {
        display.fillRect(0, 16, 128, 10, WHITE);
        display.drawLine(0, 16, 0, 63, WHITE);
        display.drawLine(127, 16, 127, 63, WHITE);
        display.drawLine(0, 63, 127, 63, WHITE);
        display.fillRect(1, 25, 126, 38, BLACK);
        display.setTextColor(BLACK);
        display.setCursor(35, 17);
        display.print("GPS INFO");
        display.setCursor(108, 17);
        display.print("5/5");
        display.setTextColor(WHITE);

        display.setCursor(3, 26);
        display.print("LAT:");
        str = String(gps.location.lat(), 5);
        x = str.length() * 6;
        display.setCursor(80 - x, 26);
        display.print(str);

        display.setCursor(3, 35);
        display.print("LON:");
        str = String(gps.location.lng(), 5);
        x = str.length() * 6;
        display.setCursor(80 - x, 35);
        display.print(str);

        display.drawYBitmap(90, 26, &Icon_TableB[50][0], 16, 16, WHITE);
        display.setCursor(110, 32);
        display.print(gps.satellites.value());

        display.setCursor(3, 44);
        display.print("SPD:");
        str = String(gps.speed.kmph(), 1) + "kph";
        /*x = str.length() * 6;
        display.setCursor(62 - x, 44);*/
        display.print(str);

        display.setCursor(80, 44);
        display.print("ALT:");
        str = String(gps.altitude.meters(), 0) + "M";
        x = str.length() * 6;
        display.setCursor(126 - x, 44);
        display.print(str);

        display.setCursor(3, 54);
        // display.print("TIME:");
        str = String(gps.date.day(), DEC) + "/" + String(gps.date.month(), DEC) + "/" + String(gps.date.year(), DEC);
        display.setCursor(3, 53);
        display.print(str);
        str = String(gps.time.hour(), DEC) + ":" + String(gps.time.minute(), DEC) + ":" + String(gps.time.second(), DEC) + "Z";
        x = str.length() * 6;
        display.setCursor(126 - x, 53);
        display.print(str);
    }
    else
    {
        // display.clearDisplay();
        display.fillRect(0, 0, 128, 64, BLACK);
        display.drawYBitmap(90, 0, &Icon_TableB[50][0], 16, 16, WHITE);
        display.setCursor(107, 7);
        display.setTextSize(1);
        display.setFont(&FreeSansBold9pt7b);
        display.print(gps.satellites.value());

        display.setCursor(0, 14);
        display.print(hour());
        display.print(":");
        display.print(minute());
        display.print(":");
        display.print(second());

        if (config.dim == 2)
        { // Auto dim timeout
            if (millis() > (dimTimeout + 60000))
            {
                display.dim(true);
            }
            else
            {
                display.dim(false);
            }
        }
        else if (config.dim == 3)
        { // Dim for time
            if (hour() > 5 && hour() < 17)
            {
                display.dim(false);
            }
            else
            {
                display.dim(true);
            }
        }

        display.setFont(&FreeSerifItalic9pt7b);
        display.setCursor(80, 28);
        display.printf("km/h");

        display.setFont(&Seven_Segment24pt7b);
        display.setCursor(70, 63);
        // display.print("188");
        display.print(SB_SPEED, DEC);
        // display.printf("ALT: %0.1fM.", gps.altitude.meters());
        // display.setFont();
        // display.setTextColor(WHITE);
        // display.setCursor(0, 0);
        // display.print(gps.satellites.value());
        ////splay.fillRect(0, 0, 128, 10, WHITE);
        // display.drawLine(0, 16, 0, 63, WHITE);

        compass_label(25, 42, 19, 0.0F, WHITE);
        compass_arrow(25, 42, 16, SB_HEADING, WHITE);
        display.drawLine(0, 16, 60, 16, WHITE);
        display.drawLine(60, 16, 70, 29, WHITE);
        display.drawLine(50, 16, 60, 29, WHITE);
        display.drawLine(60, 29, 127, 29, WHITE);
        display.setFont();
    }
    display.display();
}

void msgBox(String msg)
{
    display.fillRect(30, 26, 68, 28, BLACK);
    display.drawRect(32, 28, 64, 24, WHITE);
    display.drawLine(34, 53, 97, 53, WHITE);
    display.drawLine(97, 30, 97, 53, WHITE);
    display.setCursor(40, 37);
    display.print(msg);
    display.display();
}

uint32_t readADC_Cal(int ADC_Raw)
{
    esp_adc_cal_characteristics_t adc_chars;

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    return (esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars));
}

void topBar(int ws)
{
    // int ang = analogRead(39);
    //  float vbat;
    uint8_t vbatScal = 0;
    int wifiSignal = ws;
    uint8_t wifi = 0, i;
    int x, y;
    if (config.wifi == false)
        wifiSignal = -30;
    // display.setTextColor(WHITE);
    display.fillRect(0, 0, 128, 16, BLACK);
    // Draw Attena Signal
    display.drawTriangle(0, 0, 6, 0, 3, 3, WHITE);
    display.drawLine(3, 0, 3, 7, WHITE);
    x = 5;
    y = 3;
    wifi = (wifiSignal + 100) / 10;
    if (wifi > 5)
        wifi = 5;
    if (wifi < 0)
        wifi = 0;
    for (i = 0; i < wifi; i++)
    {
        display.drawLine(x, 7 - y, x, 7, WHITE);
        x += 2;
        y++;
    }
    // yield();
    display.setCursor(0, 8);
    if (config.wifi)
    {
        display.print(wifiSignal);
        display.print("dBm");
    }
    else
    {
        display.print("DIS");
    }

    // vbat = (ang * 2.427)/1000.0F;
    // vbat = (float)ang / 241;
    // vbat = (float)readADC_Cal(ang) / 411.5565F;
    vbat = (float)PMU.getBattVoltage() / 1000;

    x = 109;
    display.drawLine(0 + x, 1, 2 + x, 1, WHITE);
    display.drawLine(0 + x, 6, 2 + x, 6, WHITE);
    display.drawLine(0 + x, 2, 0 + x, 5, WHITE);
    display.drawLine(2 + x, 0, 18 + x, 0, WHITE);
    display.drawLine(2 + x, 7, 18 + x, 7, WHITE);
    display.drawLine(18 + x, 1, 18 + x, 6, WHITE);
    if (vbat < 3.3)
        vbatScal = 0;
    else
        vbatScal = (uint8_t)ceil((vbat - 3.3) * 6);
    // vbatScal += 1;
    if (vbatScal > 4)
        vbatScal = 5;
    x = 16 + 109;
    for (i = 0; i < vbatScal; i++)
    {
        display.drawLine(x, 2, x, 5, WHITE);
        x--;
        display.drawLine(x, 2, x, 5, WHITE);
        x -= 2;
    }
    display.setCursor(104, 8);
    display.print(vbat, 1);
    display.print("V");
    // Wifi Status
    // display.setCursor(15,0);
    // display.print("WiFi");
    if (config.wifi)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            // display.drawLine(15,0,35,8,WHITE);
            // display.setCursor(0,45);
            ////display.setTextSize(2);
            // switch(WiFi.status()){
            // case WL_IDLE_STATUS:
            //	display.println("IDLE");
            //	break;
            // case WL_NO_SSID_AVAIL:
            //	display.println("NO SSID");
            //	break;
            // case WL_SCAN_COMPLETED:
            //	display.println("SCAN COMPLETE");
            //	break;
            // case WL_CONNECTED:
            //	display.println("CONNECTED");
            //	break;
            // case WL_CONNECT_FAILED:
            //	display.println("CONNECT FAILED");
            //	break;
            // case WL_CONNECTION_LOST:
            //	display.println("CONNECTION LOST");
            //	break;
            // case WL_DISCONNECTED:
            //	display.println("DISCONNECTED");
            //	break;
            // }
            // display.setTextSize(1);
            // display.display();
            // delay(1000);
            display.fillRect(15, 0, 35, 8, BLACK);
        }
        else
        {
            display.setCursor(15, 0);
            display.print("WiFi");
        }
    }
    // DCS Status
    // display.setCursor(50,0);
    //  display.println("DCS");

    if (aprsClient.connected())
    {
        display.setCursor(50, 0);
        display.print("INET");
        // display.drawLine(50,0,65,8,WHITE);
    }

    if (gps.location.isValid())
    {
        display.setCursor(85, 0);
        display.print("GPS");
    }

    display.setCursor(110, 0);
    // if (config.tnc)
    // {
    //     if (nTNC)
    //     {
    //         display.print("TNC");
    //         // display.drawLine(50,0,65,8,WHITE);
    //     }
    //     else
    //     {
    //         display.print("NOT");
    //     }
    // }
    // else
    // {
    //     display.print("DIS");
    // }
    char strTime[10];
    struct tm tmstruct;
    tmstruct.tm_year = 0;
    getLocalTime(&tmstruct, 100);
    sprintf(strTime, "%02d:%02d:%02d", tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
    // sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);

    display.setCursor(50, 8);
    display.print(strTime);
    // display.print(hour());
    // display.print(":");
    // display.print(minute());
    // display.print(":");
    // display.print(second());

    // display.setCursor(115, 8);
    // display.print(raw_count);
    if (config.dim == 2)
    { // Auto dim timeout
        if (millis() > (dimTimeout + 60000))
        {
            display.dim(true);
        }
        else
        {
            display.dim(false);
        }
    }
    else if (config.dim == 3)
    { // Dim for time
        if (hour() > 5 && hour() < 17)
        {
            display.dim(false);
        }
        else
        {
            display.dim(true);
        }
    }

    display.display();
}

// void doEncoder() {
//	encoder_A = digitalRead(keyA);
//	if (encoder_A != (encoder_A_prev)) {
//		if (digitalRead(keyA) == HIGH) {   // found a low-to-high on channel A
//										   //delay(1);
//			if (digitalRead(keyB) == LOW) {  // check channel B to see which way
//											 // encoder is turning
//				encoder0Pos = encoder0Pos - 1;         // CCW
//			}
//			else {
//				encoder0Pos = encoder0Pos + 1;         // CW
//			}
//		}
//		else                                        // found a high-to-low on channel A
//		{
//			if (digitalRead(keyB) == LOW) {   // check channel B to see which way
//											  // encoder is turning
//				encoder0Pos = encoder0Pos + 1;          // CW
//			}
//			else {
//				encoder0Pos = encoder0Pos - 1;          // CCW
//			}
//
//		}
//		encoder_A_prev = encoder_A;     // Store value of A for next time
//										//SerialLOG.println(encoder0Pos, DEC);
//	}
// }
uint8_t KeyDelay(uint8_t pin)
{
    int8_t Key = 0;
    int i = 0;
    uint8_t ret = 0;
    do
    {
        delay(1);
        if (digitalRead(pin))
            Key++;
        else
            Key--;
    } while (++i < 10);
    if (Key > 0)
        ret = 1;
    else
        ret = 0;
    return ret;
}

const int8_t KNOBDIR[] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0};

volatile int8_t _oldState;

volatile long _position;        // Internal position (4 times _positionExt)
volatile long _positionExt;     // External position
volatile long _positionExtPrev; // External position (used only for direction checking)

portMUX_TYPE muxKey = portMUX_INITIALIZER_UNLOCKED;
unsigned long keyPeriadTime;
void IRAM_ATTR doEncoder()
{
    portENTER_CRITICAL_ISR(&muxKey);
    int sig1 = digitalRead(keyA);
    int sig2 = digitalRead(keyB);
    int8_t thisState = sig1 | (sig2 << 1);

    if (_oldState != thisState)
    {
        _position += KNOBDIR[thisState | (_oldState << 2)];
        _oldState = thisState;
        //Serial.printf("Key:%d\n", _position >> 2);
        _positionExt = _position >> 2;
        if (_positionExtPrev != _positionExt)
        {
            if (_positionExtPrev > _positionExt)
            {
                encoder0Pos--;
            }
            else if (_positionExtPrev < _positionExt)
            {
                encoder0Pos++;
            }
            _positionExtPrev = _positionExt;
            //Serial.printf("Key:%d\n", _positionExt);
        }        
    }
    portEXIT_CRITICAL_ISR(&muxKey);
}

// boolean dataAct = false;
// uint8_t x = 0, y = 0;
// char str[300];
// String callSign;
// char callDest[8];
// char path[64];
// char raw[128];
// int posNow = 0;
// int timeHalfSec = 0;

void displayInfo()
{
    SerialLOG.print(F("Location: "));
    if (gps.location.isValid())
    {
        SerialLOG.print(gps.location.lat(), 6);
        SerialLOG.print(F(","));
        SerialLOG.print(gps.location.lng(), 6);
    }
    else
    {
        SerialLOG.print(F("INVALID"));
    }

    SerialLOG.print(F("  Date/Time: "));
    if (gps.date.isValid())
    {
        SerialLOG.print(gps.date.month());
        SerialLOG.print(F("/"));
        SerialLOG.print(gps.date.day());
        SerialLOG.print(F("/"));
        SerialLOG.print(gps.date.year());
    }
    else
    {
        SerialLOG.print(F("INVALID"));
    }

    SerialLOG.print(F(" "));
    if (gps.time.isValid())
    {
        if (gps.time.hour() < 10)
            SerialLOG.print(F("0"));
        SerialLOG.print(gps.time.hour());
        SerialLOG.print(F(":"));
        if (gps.time.minute() < 10)
            SerialLOG.print(F("0"));
        SerialLOG.print(gps.time.minute());
        SerialLOG.print(F(":"));
        if (gps.time.second() < 10)
            SerialLOG.print(F("0"));
        SerialLOG.print(gps.time.second());
        SerialLOG.print(F("."));
        if (gps.time.centisecond() < 10)
            SerialLOG.print(F("0"));
        SerialLOG.print(gps.time.centisecond());
    }
    else
    {
        SerialLOG.print(F("INVALID"));
    }

    SerialLOG.println();
}

// long lastGPS = 0;

unsigned long saveTimeout = 0;
unsigned long menuTimeout = 0;
unsigned long disp_delay = 0;
uint8_t dispMode = 0;
String rawDisp;
int selTab = 0;
bool dispPush = 0;

extern unsigned long timeGui;

void mainDisp(void *pvParameters)
{
    pinMode(keyA, INPUT_PULLUP);
    pinMode(keyB, INPUT_PULLUP);
    pinMode(keyPush, INPUT_PULLUP);
    // digitalWrite(keyA, LOW);
    // digitalWrite(keyB, LOW);
    // digitalWrite(keyPush, HIGH);

    // rotaryEncoder.areEncoderPinsPulldownforEsp32 = false;
    // rotaryEncoder.begin();
    // rotaryEncoder.setup(readEncoderISR);
    // rotaryEncoder.setBoundaries(1, 5, true); // minValue, maxValue, circleValues true|false (when max go to min and vice versa)
    // rotaryEncoder.setAcceleration(250);

    // pinMode(keyA, INPUT_PULLUP);
    // pinMode(keyB, INPUT_PULLUP);

    // topBar(-100);
    conStatNetwork = CON_WIFI;
    conStat = CON_NORMAL;

    // showDisp=false;
    curTab = 3;
    // oledSleepTimeout = millis() + (config.oled_timeout * 1000);

    mnuAbout.add_item(&mnuAbout_mi1);
    mnuAbout.add_item(&mnuAbout_mi2);
    mnuAbout.add_item(&mnuAbout_mi3);
    mnuAbout.add_item(&mnuAbout_mi4);
    // ms.get_root_menu().add_menu(&mnuConfig); //Wiress
    mnuConfig.add_item(&mnuConfig_mi1);
    mnuConfig.add_item(&mnuConfig_mi2);
    mnuConfig.add_item(&mnuConfig_mi3);

    ms.get_root_menu().add_menu(&mnu1); // Wiress
    mnu1.add_item(&mnu1_mi1);
    mnu1.add_item(&mnu1_mi2);
    mnu1.add_item(&mnu1_mi3);
    mnu1.add_item(&mnu1_mi4);

    ms.get_root_menu().add_menu(&mnu2); // IGATE
    mnu2.add_item(&mnu2_mi1);
    mnu2.add_item(&mnu2_mi2);
    mnu2.add_item(&mnu2_mi3);
    mnu2.add_item(&mnu2_mi4);

    ms.get_root_menu().add_menu(&mnu3); // TRACKER
    mnu3.add_item(&mnu3_mi1);
    mnu3.add_item(&mnu3_mi2);
    mnu3.add_item(&mnu3_mi3);
    mnu3.add_item(&mnu3_mi4);

    ms.get_root_menu().add_menu(&mnu4); // DIGI
    mnu4.add_item(&mnu4_mi1);
    mnu4.add_item(&mnu4_mi2);
    mnu4.add_item(&mnu4_mi3);
    mnu4.add_item(&mnu4_mi4);

    ms.get_root_menu().add_menu(&mnu5); // SYSTEM
    mnu5.add_menu(&mnuConfig);
    mnu5.add_item(&mnu5_mi2);
    mnu5.add_item(&mnu5_mi3);
    mnu5.add_menu(&mnuAbout);

    // ms.get_root_menu().add_menu(&mu1);
    // mu1.add_item(&mu1_mi1);
    // mu1.add_item(&mu1_mi2);
    // mu1.add_item(&mu1_mi3);
    // mu1.add_item(&mu1_mi4);
    // ms.get_root_menu().add_menu(&mu2);
    // mu2.add_item(&mu2_mi1);
    // mu2.add_item(&mu2_mi2);
    // mu2.add_item(&mu2_mi3);
    // mu2.add_item(&mu2_mi4);
    // mu2.add_item(&mu2_mi5);

    // ms.get_root_menu().add_menu(&mu6);
    // mu6.add_item(&mu6_mi0);
    // mu6.add_item(&mu6_mi1);
    // mu6.add_item(&mu6_mi2);
    // // mu6.add_item(&mu6_mi3);

    // ms.get_root_menu().add_menu(&mu5);
    // mu5.add_item(&mu5_mi1);
    // mu5.add_item(&mu5_mi2);
    // mu5.add_item(&mu5_mi3);

    // ms.get_root_menu().add_menu(&mu4);
    // mu4.add_item(&mu4_mi1);
    // mu4.add_item(&mu4_mi2);
    // mu4.add_item(&mu4_mi3);
    // mu4.add_item(&mu4_mi4);
    // //mu4.add_item(&mu4_mi5);
    // mu4.add_menu(&mu3);

    // // mu4.add_item(&mu4_mi6);
    // ms.get_root_menu().add_menu(&mu3);
    // mu3.add_item(&mu3_mi1);
    // mu3.add_item(&mu3_mi2);
    // mu3.add_item(&mu3_mi3);
    // mu3.add_item(&mu3_mi4);
    // // ms.display();

    // rotaryEncoder.enable();

    attachInterrupt(keyA, doEncoder, CHANGE);
    attachInterrupt(keyB, doEncoder, CHANGE);

    if (config.startup > 5)
        config.startup = 0;
    if (config.startup < 5)
    {
        curTab = config.startup + 1;
        gps_mode = 0;
    }
    else
    {
        curTab = 5;
        gps_mode = 1;
    }
    topBar(-1);

    if (config.mygps == false)
    {
        tx_counter = 0;
        tx_interval = 10;
    }

    unsigned long timeGuiOld = millis();
    timeGui = 0;
    saveTimeout = millis();
    for (;;)
    {
        unsigned long now = millis();
        timeGui = now - timeGuiOld;
        timeGuiOld = now;
        vTaskDelay(10 / portTICK_PERIOD_MS);

        if (millis() > (saveTimeout + 300000))
        {
            powerSave();
        }

        if (conStat == CON_NORMAL)
        {
            menuTimeout = millis();
            // readSerialGPS();
            if ((raw_count > 0) && (disp_delay == 0))
            {
                saveTimeout = millis();
                dispPush = false;
                popTNC2Raw(selTab);
                rawDisp = String(pkgList[selTab].raw);
                dispWindow(rawDisp, dispMode, true);
                // selTab = 1;
            }
            // if (raw_count > 0)
            //{
            if (dispFlagTX == 1)
            {
                dispTX(1);
                dispFlagTX = 0;
            }
            else if (dispFlagTX == 2)
            {
                dispTX(0);
                dispFlagTX = 0;
            }
            //}

            if (millis() > (unsigned long)timeHalfSec)
            {
                timeHalfSec = millis() + 500 + disp_delay;
                // powerWakeup();
                disp_delay = 0;
                dispMode = 0;
                // dispFlagTX=0;
                if (powerStatus() && (raw_count == 0))
                {
                    if (!(curTab == 5 && gps_mode == 1))
                        topBar(WiFi.RSSI());
                    switch (curTab)
                    {
                    case 1:
                        statisticsDisp();
                        break;
                    case 2:
                        pkgLastDisp();
                        break;
                    case 3:
                        pkgCountDisp();
                        break;
                    case 4:
                        systemDisp();
                        break;
                    case 5:
                        gpsDisp();
                        break;
                    }
                }
            }
            else if (disp_delay > 0)
            {
                if (encoder0Pos != posNow)
                {
                    saveTimeout = millis();
                    if (config.dim == 2)
                        dimTimeout = millis();
                    if (encoder0Pos > posNow)
                    {
                        selTab++;
                        for (; selTab < PKGLISTSIZE; selTab++)
                        {
                            if (pkgList[selTab].time > 0)
                                break;
                        }
                        if (selTab >= PKGLISTSIZE)
                            selTab = 0;
                    }
                    else
                    {
                        selTab--;
                        for (; selTab >= 0; selTab--)
                        {
                            if (pkgList[selTab].time > 0)
                                break;
                        }
                        if (selTab < 0)
                            selTab = PKGLISTSIZE - 1;
                    }
                    posNow = encoder0Pos;
                    if (pkgList[selTab].time > 0)
                    {
                        rawDisp = String(pkgList[selTab].raw);
                        dispWindow(rawDisp, dispMode, false);
                    }
                }
            }

            if (raw_count == 0)
            {
                // lets see if anything changed
                // if (rotaryEncoder.encoderChanged() != 0)
                // {
                //     timeHalfSec = 0;
                //     powerWakeup();
                //     saveTimeout = millis();
                //     if (config.dim == 2)
                //         dimTimeout = millis();
                //     // now we need current value
                //     curTab = (char)rotaryEncoder.readEncoder();
                //     // process new value. Here is simple output.
                //     //  Serial.print("Value: ");
                //     //  Serial.println(curTab,DEC);
                // }
                if (encoder0Pos != posNow)
                {
                    timeHalfSec = 0;
                    powerWakeup();
                    saveTimeout = millis();
                    if (config.dim == 2)
                        dimTimeout = millis();
                    if (encoder0Pos > posNow)
                    {
                        curTab++;
                        if (curTab > 5)
                            curTab = 1;
                    }
                    else
                    {
                        curTab--;
                        if (curTab < 1)
                            curTab = 5;
                    }
                    posNow = encoder0Pos;
                }
            }
        }

        if ((digitalRead(keyPush) == LOW) && (conStat != CON_MENU))
        {
            saveTimeout = millis();
            powerWakeup();
            currentTime = millis();
            delay(500);
            // conStat = CON_MENU;
            // TaskGPS.Enable(false);
            if (digitalRead(keyPush) == HIGH)
            { // ONE Click
                if (config.dim == 2)
                    dimTimeout = millis();
                if (disp_delay > 0)
                { // Select MODE Decode/RAW
                    dispPush = false;
                    disp_delay = config.dispDelay * 1000;
                    timeHalfSec = millis() + disp_delay;
                    if (dispMode == 0)
                        dispMode = 1;
                    else
                        dispMode = 0;
                    dispWindow(rawDisp, dispMode, false);
                }
                else
                {
                    if (curTab == 2 || curTab == 3)
                    { // To Display mode.
                        dispMode = 0;
                        selTab = 0;
                        rawDisp = String(pkgList[selTab].raw);
                        dispWindow(rawDisp, dispMode, false);
                        posNow = encoder0Pos;
                    }
                    else if (curTab == 1)
                    {
                        // EVENT_TX_POSITION = 1;
                        tx_counter = 10;
                    }
                    else if (curTab == 5)
                    {
                        if (gps_mode == 0)
                            gps_mode = 1;
                        else
                            gps_mode = 0;
                    }
                }
            }
            // ms.reset();
            // ms.display();
            while (digitalRead(keyPush) == LOW)
            {
                delay(10);
                if ((millis() - currentTime) > 2000)
                {
                    // if ((curTab == 2 || curTab == 3)&& disp_delay>0) {
                    if (disp_delay > 0)
                    {
                        dispPush = true;
                        disp_delay = 600 * 1000;
                        dispWindow(rawDisp, dispMode, false);
                    }
                    else
                    {
                        conStat = CON_MENU;
                        // TaskGPS.Enable(false);

                        ms.reset();
                        ms.display();

                        // TaskGPS.Enable(true);
                    }
                    break;
                }
            };
            while (digitalRead(keyPush) == LOW)
                delay(10);
        }

        if (conStat == CON_MENU)
        {
            delay(10);
            if (millis() > (menuTimeout + 60000L))
            {
                menuTimeout = millis();
                conStat = CON_NORMAL;
                /*if (WiFi.status() == WL_CONNECTED) {
                    conStat = CON_SERVER;
                    topBar(WiFi.RSSI());
                }
                else {
                    conStat = CON_WIFI;
                }*/
                // ESP.restart();
                // display.clearDisplay();
                // display.display();
                // if (config.tnc_init) tncInit();
                powerSave();
            }

            if (encoder0Pos != posNow)
            {
                saveTimeout = millis();
                menuTimeout = millis();
                line = 15; // line variable reset
                if (encoder0Pos > posNow)
                {
                    ms.next();
                    // ms.display();
                }
                else
                {
                    ms.prev();
                    // ms.display();
                }
                ms.display();
                posNow = encoder0Pos;
            }
            else
            {
                if ((digitalRead(keyPush) == LOW))
                {
                    saveTimeout = millis();
                    menuTimeout = millis();
                    currentTime = millis();
                    line = 15; // line variable reset
                    while (digitalRead(keyPush) == LOW)
                    {
                        delay(10);
                        if ((millis() - currentTime) > 1500)
                            break;
                    };
                    if ((millis() - currentTime) > 1000)
                    {
                        ms.back();
                    }
                    else
                    {
                        ms.select();
                    }
                    ms.display();
                    while (digitalRead(keyPush) == LOW)
                    {
                        delay(10);
                    }
                    menuTimeout = millis();
                }
            }
            // ms.display();
            // topBar(WiFi.RSSI());
        }

        //	else if (conStat == CON_WIFI) {
        //		topBar(WiFi.RSSI());
        // #ifdef DEBUG
        //		SerialLOG.println("WiFi Connecting");
        // #endif
        //		WiFi.disconnect(false);
        //		delay(500);
        //		WiFi.mode(WIFI_STA);
        //		WiFi.begin(config.wifi_ssid, config.wifi_password);
        //		display.fillRect(0, 16, 128, 48, BLACK);
        //		display.setCursor(0, 16);
        //		display.println("WiFi Connecting...");
        //		display.print("SSID: ");
        //		display.println((char*)& config.wifi_ssid);
        //		//display.display();
        //		//display.println("");
        //		display.display();
        //		//WiFi.disconnect();
        //		//WiFi.begin((char*)&ssid[current_wifi], (char*)&password[current_wifi]);         //�������͡Ѻ AP
        //		//topbar_timeout = 0;
        //		conStat = CON_SERVER;
        //		//aprsWdt = 0;
        //		aprsWdt = millis() + 20000;
        //		aprsRetry = 3;
        //		firstWiFiConnect = false;
        //		client.flush();
        //		Udp.flush();
        //		//yield();
        //		//showCallTimeout = millis() + 10000;
        //	}
        //	else if (conStat == CON_SERVER) {
        //		if (WiFi.status() == WL_CONNECTED) {
        //			topBar(WiFi.RSSI());
        // #ifdef DEBUG
        //			SerialLOG.println("WiFi Connected");
        // #endif
        //			display.println("APRS-IS Connecting..");
        //			display.display();
        //			//firstWiFiConnect = true;
        //			SerialLOG.print("IP number assigned by DHCP is ");
        //			SerialLOG.println(WiFi.localIP());
        //			if (firstWiFiConnect == 0) {
        //				firstWiFiConnect = true;
        //				SerialLOG.println("Starting UDP");
        //				Udp.flush();
        //				Udp.stopAll();
        //				delay(500);
        //				Udp.begin(localPort);
        //				SerialLOG.print("UDP Local port: ");
        //				SerialLOG.println(Udp.localPort());
        //				SerialLOG.println("waiting for sync");
        //				setSyncProvider(getNtpTime);
        //				setSyncInterval(300);
        //			}
        //			if (Client_Connect()) {
        // #ifdef DEBUG
        //				//SerialLOG.print(client.status());
        //				SerialLOG.println("APRS-IS Connected");
        // #endif
        //				conStat = CON_NORMAL;
        //				curTab = 1;
        //				TaskGPS.Enable(true);
        //				//timerBeacon = millis();
        //			}
        //			else {
        // #ifdef DEBUG
        //				//SerialLOG.println(str_status[client.status()]);
        //				SerialLOG.println("APRS-IS Connect Fail!");
        // #endif
        //				display.println("APRS-IS Fail!");
        //				display.display();
        //				//client.flush();
        //				//client.clearWriteError();
        //				//
        //				//client.stop();
        //				//if (aprsRetry-- == 0) {
        //				//	conStat = CON_WIFI;
        //				//	WiFi.disconnect(true);
        //				//	client.stopAll();
        //				//	Udp.stopAll();
        //				//	delay(500);
        //				//}
        //			}
        //		}
        //	}
        //	else {
        //
        //		if (WiFi.status() == WL_CONNECTED) {
        //			if (Client_Connect()) {
        //				if (client.available())              //ตรวจเช็คว่ามีการส่งค่ากลับมาจาก Server หรือไม่
        //				{
        //					aprsWdt = millis() + 30000;
        //					do {
        //						String line = client.readStringUntil('\n');       //อ่านค่าที่ Server ตอบหลับมาทีละบรรทัด
        // #ifdef DEBUG
        //						printTime();
        //						SerialLOG.print("APRS-IS ");
        //						SerialLOG.println(line);
        // #endif
        //						status.isCount++;
        //						int start_val = line.indexOf(">", 0); // หาตำแหน่งแรกของ >
        //						if (start_val > 3) {
        //							String src_call = line.substring(0, start_val);
        //							String msg_call = "::" + src_call;
        //							status.allCount++;
        //							digiTLM.RX++;
        //							if (config.inet2rf) {
        //								if (line.indexOf(msg_call) <= 0) {
        //									raw[0] = '}';
        //									line.toCharArray(&raw[1], line.length());
        //									SerialTNC.println(raw);
        //									status.inet2rf++;
        //									digiTLM.INET2RF++;
        //									printTime();
        //									SerialLOG.print("INET2RF ");
        //									SerialLOG.println(raw);
        //								}
        //								else {
        //									digiTLM.DROP++;
        //									SerialLOG.print("INET2RF IS Message TELEMETRY from ");
        //									SerialLOG.println(src_call);
        //								}
        //							}
        //
        //							memset(&raw[0], 0, sizeof(raw));
        //							line.toCharArray(&raw[0], start_val + 1);
        //							raw[start_val + 1] = 0;
        //							pkgListUpdate(&raw[0], 0);
        //						}
        //						//else {
        //						//	status.errorCount++;
        //						//}
        //					} while (client.available());
        //
        //					if (config.mylocat) {
        //						if (timerBeacon < millis()) {
        //							timerBeacon = millis() + 600000; //10Min
        //							if ((config.mygps == true && gps.location.isValid() == true)||config.mygps==false) {
        //								status.isCount++;
        //								digiTLM.TX++;
        //								String raw = myBeacon();
        //								client.println(raw);
        // #ifdef DEBUG
        //								printTime();
        //								SerialLOG.print("BEACON ");
        //								SerialLOG.println(raw);
        // #endif
        //							}else {
        //								timerBeacon = millis() + 60000; //1Min
        //							}
        //						}
        //					}
        //					if (config.mytelemetry) {
        //						if (digiTLM.TeleTimeout < millis()) {
        //							digiTLM.TeleTimeout = millis() + 600000; //10Min
        //							if ((digiTLM.Sequence % 6) == 0) {
        //								sendIsPkgMsg((char*)& PARM[0]);
        //								sendIsPkgMsg((char*)& UNIT[0]);
        //								sendIsPkgMsg((char*)& EQNS[0]);
        //							}
        //							sprintf(raw, "T#%03d,%d,%d,%d,%d,%d,00000000", digiTLM.Sequence, digiTLM.RF2INET, digiTLM.INET2RF, digiTLM.RX, digiTLM.TX, digiTLM.DROP);
        //							sendIsPkg(raw);
        //							digiTLM.Sequence++;
        //							if (digiTLM.Sequence > 999) digiTLM.Sequence = 0;
        //							digiTLM.DROP = 0;
        //							digiTLM.INET2RF = 0;
        //							digiTLM.RF2INET = 0;
        //							digiTLM.RX = 0;
        //							digiTLM.TX = 0;
        //							//client.println(raw);
        //						}
        //					}
        //					client.flush();
        //				}
        //
        //				if (config.rf2inet) {
        //					if (SerialTNC.available() > 0) {
        //						status.allCount++;
        //						String tnc2 = SerialTNC.readStringUntil('\n');
        //						int start_val = tnc2.indexOf(">", 0); // หาตำแหน่งแรกของ >
        //						if (start_val > 3) {
        //							status.tncCount++;
        //							if (tnc2.indexOf("RFONLY", 10) > 0) {
        //								status.dropCount++;
        //								digiTLM.DROP++;
        //							}
        //							else {
        //								tnc2.toCharArray(&str[0], tnc2.length());
        //								int i = tnc2.indexOf(":");
        //								if (i > 10) {
        //									str[i] = 0;
        //									sprintf(raw, "%s,qAR,%s-%d:%s", &str[0], config.mycallsign, config.myssid, &str[i + 1]);
        //									tnc2 = String(raw);
        //									client.println(tnc2);
        //									status.rf2inet++;
        //									digiTLM.RF2INET++;
        //									digiTLM.TX++;
        //									printTime();
        //									SerialLOG.print("RF2INET ");
        //									SerialLOG.println(raw);
        //								}
        //								else {
        //									status.errorCount++;
        //									digiTLM.DROP++;
        //								}
        //							}
        //							memset(&raw[0], 0, sizeof(raw));
        //							tnc2.toCharArray(&raw[0], start_val + 1);
        //							raw[start_val + 1] = 0;
        //							pkgListUpdate(&raw[0], 1);
        // #ifdef DEBUG
        //							printTime();
        //							SerialLOG.print("TNC ");
        //							SerialLOG.println(tnc2);
        // #endif
        //						}
        //						else { status.errorCount++; }
        //
        //					}
        //				}
        //			}
        //			else {
        // #ifdef DEBUG
        //				//SerialLOG.print(client.status());
        //				SerialLOG.println("APRS-IS Disconnected");
        // #endif
        //				display.fillRect(0, 16, 128, 48, BLACK);
        //				display.setCursor(0, 16);
        //				display.println("APRS-IS");
        //				display.println("DISCONNECT");
        //				display.display();
        //				client.stop();
        //				//yield();
        //				delay(500);
        //			}
        //			if (millis() > aprsWdt) {
        // #ifdef DEBUG
        //				//SerialLOG.print(client.status());
        //				SerialLOG.println("APRS-IS Timeout!");
        // #endif
        //				client.flush();
        //				client.clearWriteError();
        //				delay(500);
        //				client.stop();
        //				conStat = CON_SERVER;
        //				aprsRetry = 3;
        //				delay(500);
        //			}
        //		}
        //		else {
        // #ifdef DEBUG
        //			SerialLOG.print(client.status());
        //			SerialLOG.println(":WiFi Disconnected");
        // #endif
        //			//WiFi Connected
        //			conStat = CON_WIFI;
        //			client.stopAll();
        //			Udp.stopAll();
        //			WiFi.disconnect(false);
        //			//yield();
        //			delay(500);
        //		}
        //	}
    }
}

// Routine
void line_angle(signed int startx, signed int starty, unsigned int length, unsigned int angle, unsigned int color)
{
    display.drawLine(startx, starty, (startx + length * cosf(angle * 0.017453292519)), (starty + length * sinf(angle * 0.017453292519)), color);
}

int xSpiGlcdSelFontHeight = 8;
int xSpiGlcdSelFontWidth = 5;

void compass_label(signed int startx, signed int starty, unsigned int length, double angle, unsigned int color)
{
    double angleNew;
    // ushort Color[2];
    uint8_t x_N, y_N, x_S, y_S;
    int x[4], y[4], i;
    int xOffset, yOffset;
    yOffset = (xSpiGlcdSelFontHeight / 2);
    xOffset = (xSpiGlcdSelFontWidth / 2);
    // GLCD_WindowMax();
    angle += 270.0F;
    angleNew = angle;
    for (i = 0; i < 4; i++)
    {
        if (angleNew > 360.0F)
            angleNew -= 360.0F;
        x[i] = startx + (length * cosf(angleNew * 0.017453292519));
        y[i] = starty + (length * sinf(angleNew * 0.017453292519));
        x[i] -= xOffset;
        y[i] -= yOffset;
        angleNew += 90.0F;
    }
    angleNew = angle + 45.0F;
    for (i = 0; i < 4; i++)
    {
        if (angleNew > 360.0F)
            angleNew -= 360.0F;
        x_S = startx + ((length - 3) * cosf(angleNew * 0.017453292519));
        y_S = starty + ((length - 3) * sinf(angleNew * 0.017453292519));
        x_N = startx + ((length + 3) * cosf(angleNew * 0.017453292519));
        y_N = starty + ((length + 3) * sinf(angleNew * 0.017453292519));
        angleNew += 90.0F;
        display.drawLine(x_S, y_S, x_N, y_N, color);
    }
    display.drawCircle(startx, starty, length, color);
    display.setFont();
    display.drawChar((uint8_t)x[0], (uint8_t)y[0], 'N', WHITE, BLACK, 1);
    display.drawChar((uint8_t)x[1], (uint8_t)y[1], 'E', WHITE, BLACK, 1);
    display.drawChar((uint8_t)x[2], (uint8_t)y[2], 'S', WHITE, BLACK, 1);
    display.drawChar((uint8_t)x[3], (uint8_t)y[3], 'W', WHITE, BLACK, 1);
}

void compass_arrow(signed int startx, signed int starty, unsigned int length, double angle, unsigned int color)
{
    double angle1, angle2;
    int xdst, ydst, x1sta, y1sta, x2sta, y2sta;
    int length2 = length / 2;
    angle += 270.0F;
    if (angle > 360.0F)
        angle -= 360.0F;
    xdst = startx + length * cosf(angle * 0.017453292519);
    ydst = starty + length * sinf(angle * 0.017453292519);
    angle1 = angle + 135.0F;
    if (angle1 > 360.0F)
        angle1 -= 360.0F;
    angle2 = angle + 225.0F;
    if (angle2 > 360.0F)
        angle2 -= 360.0F;
    x1sta = startx + length2 * cosf(angle1 * 0.017453292519);
    y1sta = starty + length2 * sinf(angle1 * 0.017453292519);
    x2sta = startx + length2 * cosf(angle2 * 0.017453292519);
    y2sta = starty + length2 * sinf(angle2 * 0.017453292519);
    display.drawLine(startx, starty, xdst, ydst, color);
    display.drawLine(xdst, ydst, x1sta, y1sta, color);
    display.drawLine(x1sta, y1sta, startx, starty, color);
    display.drawLine(startx, starty, x2sta, y2sta, color);
    display.drawLine(x2sta, y2sta, xdst, ydst, color);
}

void dispTX(bool port)
{

    display.clearDisplay();
    // display.fillRect(0, 0, 128, 64, BLACK);
    disp_delay = 2000;
    timeHalfSec = millis() + disp_delay;
    // display.fillRect(0, 0, 128, 16, WHITE);
    const uint8_t *ptrSymbol;
    uint8_t symIdx = send_aprs_symbol - 0x21;
    if (symIdx > 95)
        symIdx = 0;
    if (send_aprs_table == '/')
    {
        ptrSymbol = &Icon_TableA[symIdx][0];
    }
    else if (send_aprs_table == '\\')
    {
        ptrSymbol = &Icon_TableB[symIdx][0];
    }
    else
    {
        if (send_aprs_table < 'A' || send_aprs_table > 'Z')
        {
            send_aprs_table = 'N';
            send_aprs_symbol = '&';
            symIdx = 5; // &
        }
        ptrSymbol = &Icon_TableB[symIdx][0];
    }
    display.drawYBitmap(0, 0, ptrSymbol, 16, 16, WHITE);
    if (!(send_aprs_table == '/' || send_aprs_table == '\\'))
    {
        display.drawChar(5, 4, send_aprs_table, BLACK, WHITE, 1);
        display.drawChar(6, 5, send_aprs_table, BLACK, WHITE, 1);
    }

    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(20, 14);
    if (strlen(config.tnc_item))
    {
        display.print(config.tnc_item);
    }
    else
    {
        display.print(config.aprs_mycall);
        if (config.aprs_ssid > 0)
        {
            display.print("-");
            display.print(config.aprs_ssid);
        }
    }

    display.setFont(&FreeSerifItalic9pt7b);

    if (port == 0)
    {
        display.setCursor(5, 42);
        display.print("TCP");
        display.setCursor(15, 57);
        display.print("IP");
    }
    else
    {
        display.setCursor(3, 42);
        display.print("SEND");
        display.setCursor(15, 57);
        display.print("RF");
    }

    display.setFont();
    display.setTextColor(WHITE);
    // if (selTab < 10)
    //	display.setCursor(121, 0);
    // else
    display.setCursor(115, 0);
    display.print("TX");

    display.drawRoundRect(0, 16, 128, 48, 5, WHITE);
    display.fillRoundRect(1, 17, 126, 10, 2, WHITE);
    display.setTextColor(BLACK);
    display.setCursor(40, 18);
    display.print("TX STATUS");

    display.setTextColor(WHITE);
    display.setCursor(50, 30);
    if (config.mygps)
    {
        display.printf("POSITION GPS");
        // display.setCursor(48, 39);
        // display.printf("HEADING %d", SB_HEADING);
        display.setCursor(50, 48);
        display.printf("SPD %dkPh/%d", SB_SPEED, SB_HEADING);
    }
    else
    {
        display.printf("POSITION FIX");
    }
    display.setCursor(50, 39);
    display.printf("INTERVAL %dS", tx_interval);
    // display.setCursor(48, 39);
    // display.printf("HEADING %d", SB_HEADING);
    // display.setCursor(48, 48);
    // display.printf("SPEED %dkm/h", SB_SPEED);

    display.display();
}

// char* directions[] = { "S", "SW", "W", "NW", "N", "NE", "E", "SE", "S" };
const char *directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

void dispWindow(String line, uint8_t mode, bool filter)
{
    struct pbuf_t aprs;
    uint16_t bgcolor, txtcolor;
    bool Monitor = false;
    char text[200];
    unsigned char x = 0;
    char itemname[10];
    int start_val = line.indexOf(">", 0); // หาตำแหน่งแรกของ >
    if (start_val > 3)
    {
        powerWakeup();
        // Serial.println(line);
        String src_call = line.substring(0, start_val);
        memset(&aprs, 0, sizeof(pbuf_t));
        aprs.buf_len = 300;
        aprs.packet_len = line.length();
        line.toCharArray(&aprs.data[0], aprs.packet_len);
        int start_info = line.indexOf(":", 0);
        int end_ssid = line.indexOf(",", 0);
        int start_dst = line.indexOf(">", 2);
        int start_dstssid = line.indexOf("-", start_dst);
        if ((start_dstssid > start_dst) && (start_dstssid < start_dst + 10))
        {
            aprs.dstcall_end_or_ssid = &aprs.data[start_dstssid];
        }
        else
        {
            aprs.dstcall_end_or_ssid = &aprs.data[end_ssid];
        }
        aprs.info_start = &aprs.data[start_info + 1];
        aprs.dstname = &aprs.data[start_dst + 1];
        aprs.dstname_len = end_ssid - start_dst;
        aprs.dstcall_end = &aprs.data[end_ssid];
        aprs.srccall_end = &aprs.data[start_dst];

        // Serial.println(aprs.info_start);
        // aprsParse.parse_aprs(&aprs);
        if (aprsParse.parse_aprs(&aprs))
        {
            if (filter == true)
            {
                if (config.filterStatus && (aprs.packettype & T_STATUS))
                {
                    Monitor = true;
                }
                else if (config.filterMessage && (aprs.packettype & T_MESSAGE))
                {
                    Monitor = true;
                }
                else if (config.filterTelemetry && (aprs.packettype & T_TELEMETRY))
                {
                    Monitor = true;
                }
                else if (config.filterWeather && ((aprs.packettype & T_WX) || (aprs.packettype & T_WAVE)))
                {
                    Monitor = true;
                }

                if (config.filterPosition && (aprs.packettype & T_POSITION))
                {
                    double lat, lon;
                    if ((config.mygps == true) && gps.location.isValid())
                    {
                        lat = gps.location.lat();
                        lon = gps.location.lng();
                    }
                    else
                    {
                        lat = config.gps_lat;
                        lon = config.gps_lon;
                    }
                    double dist = aprsParse.distance(lon, lat, aprs.lng, aprs.lat);
                    if (config.filterDistant == 0)
                    {
                        Monitor = true;
                    }
                    else
                    {
                        if (dist < config.filterDistant)
                            Monitor = true;
                        else
                            Monitor = false;
                    }
                }

                if (config.filterTracker && (aprs.packettype & T_POSITION))
                {
                    if (aprs.flags & F_CSRSPD)
                    {
                        double lat, lon;
                        if ((config.mygps == true) && gps.location.isValid())
                        {
                            lat = gps.location.lat();
                            lon = gps.location.lng();
                        }
                        else
                        {
                            lat = config.gps_lat;
                            lon = config.gps_lon;
                        }
                        double dist = aprsParse.distance(lon, lat, aprs.lng, aprs.lat);
                        if (config.filterDistant == 0)
                        {
                            Monitor = true;
                        }
                        else
                        {
                            if (dist < config.filterDistant)
                                Monitor = true;
                            else
                                Monitor = false;
                        }
                    }
                }

                if (config.filterMove && (aprs.packettype & T_POSITION))
                {
                    if (aprs.flags & F_CSRSPD)
                    {
                        if (aprs.speed > 0)
                        {
                            double lat, lon;
                            if ((config.mygps == true) && gps.location.isValid())
                            {
                                lat = gps.location.lat();
                                lon = gps.location.lng();
                            }
                            else
                            {
                                lat = config.gps_lat;
                                lon = config.gps_lon;
                            }
                            double dist = aprsParse.distance(lon, lat, aprs.lng, aprs.lat);
                            if (config.filterDistant == 0)
                            {
                                Monitor = true;
                            }
                            else
                            {
                                if (dist < config.filterDistant)
                                    Monitor = true;
                                else
                                    Monitor = false;
                            }
                        }
                    }
                }
            }
            else
            {
                Monitor = true;
            }
        }
        else
        {
            return;
        }

        if (Monitor)
        {
            display.clearDisplay();
            if (dispPush)
            {
                disp_delay = 600 * 1000;
                display.drawRoundRect(0, 0, 128, 16, 5, WHITE);
            }
            else
            {
                disp_delay = config.dispDelay * 1000;
            }
            timeHalfSec = millis() + disp_delay;
            // display.fillRect(0, 0, 128, 16, WHITE);
            const uint8_t *ptrSymbol;
            uint8_t symIdx = aprs.symbol[1] - 0x21;
            if (symIdx > 95)
                symIdx = 0;
            if (aprs.symbol[0] == '/')
            {
                ptrSymbol = &Icon_TableA[symIdx][0];
            }
            else if (aprs.symbol[0] == '\\')
            {
                ptrSymbol = &Icon_TableB[symIdx][0];
            }
            else
            {
                if (aprs.symbol[0] < 'A' || aprs.symbol[0] > 'Z')
                {
                    aprs.symbol[0] = 'N';
                    aprs.symbol[1] = '&';
                    symIdx = 5; // &
                }
                ptrSymbol = &Icon_TableB[symIdx][0];
            }
            display.drawYBitmap(0, 0, ptrSymbol, 16, 16, WHITE);
            if (!(aprs.symbol[0] == '/' || aprs.symbol[0] == '\\'))
            {
                display.drawChar(5, 4, aprs.symbol[0], BLACK, WHITE, 1);
                display.drawChar(6, 5, aprs.symbol[0], BLACK, WHITE, 1);
            }
            display.setCursor(20, 7);
            display.setTextSize(1);
            display.setFont(&FreeSansBold9pt7b);

            if (aprs.srcname_len > 0)
            {
                memset(&itemname, 0, sizeof(itemname));
                memcpy(&itemname, aprs.srcname, aprs.srcname_len);
                Serial.println(itemname);
                display.print(itemname);
            }
            else
            {
                display.print(src_call);
            }

            display.setFont();
            display.setTextColor(WHITE);
            if (selTab < 10)
                display.setCursor(121, 0);
            else
                display.setCursor(115, 0);
            display.print(selTab);

            if (mode == 1)
            {
                display.drawRoundRect(0, 16, 128, 48, 5, WHITE);
                display.fillRoundRect(1, 17, 126, 10, 2, WHITE);
                display.setTextColor(BLACK);
                display.setCursor(40, 18);
                display.print("TNC2 RAW");

                display.setFont();
                display.setCursor(2, 30);
                display.setTextColor(WHITE);
                display.print(line);

                display.display();
                return;
            }

            if (aprs.packettype & T_TELEMETRY)
            {
                bool show = false;
                int idx = tlmList_Find((char *)src_call.c_str());
                if (idx < 0)
                {
                    idx = tlmListOld();
                    if (idx > -1)
                        memset(&Telemetry[idx], 0, sizeof(Telemetry_struct));
                }
                if (idx > -1)
                {
                    Telemetry[idx].time = now();
                    strcpy(Telemetry[idx].callsign, (char *)src_call.c_str());

                    // for (int i = 0; i < 3; i++) Telemetry[idx].UNIT[i][5] = 0;
                    if (aprs.flags & F_UNIT)
                    {
                        memcpy(Telemetry[idx].UNIT, aprs.tlm_unit.val, sizeof(Telemetry[idx].UNIT));
                    }
                    else if (aprs.flags & F_PARM)
                    {
                        memcpy(Telemetry[idx].PARM, aprs.tlm_parm.val, sizeof(Telemetry[idx].PARM));
                    }
                    else if (aprs.flags & F_EQNS)
                    {
                        for (int i = 0; i < 15; i++)
                            Telemetry[idx].EQNS[i] = aprs.tlm_eqns.val[i];
                    }
                    else if (aprs.flags & F_BITS)
                    {
                        Telemetry[idx].BITS_FLAG = aprs.telemetry.bitsFlag;
                    }
                    else if (aprs.flags & F_TLM)
                    {
                        for (int i = 0; i < 5; i++)
                            Telemetry[idx].VAL[i] = aprs.telemetry.val[i];
                        Telemetry[idx].BITS = aprs.telemetry.bits;
                        show = true;
                    }

                    for (int i = 0; i < 4; i++)
                    { // Cut length
                        if (strstr(Telemetry[idx].PARM[i], "RxTraffic") != 0)
                            sprintf(Telemetry[idx].PARM[i], "RX");
                        if (strstr(Telemetry[idx].PARM[i], "TxTraffic") != 0)
                            sprintf(Telemetry[idx].PARM[i], "TX");
                        if (strstr(Telemetry[idx].PARM[i], "RxDrop") != 0)
                            sprintf(Telemetry[idx].PARM[i], "DROP");
                        Telemetry[idx].PARM[i][6] = 0;
                        Telemetry[idx].UNIT[i][3] = 0;
                        for (int a = 0; a < 3; a++)
                        {
                            if (Telemetry[idx].UNIT[i][a] == '/')
                                Telemetry[idx].UNIT[i][a] = 0;
                        }
                    }

                    for (int i = 0; i < 5; i++)
                    {
                        if (Telemetry[idx].PARM[i][0] == 0)
                        {
                            sprintf(Telemetry[idx].PARM[i], "CH%d", i + 1);
                        }
                    }
                }
                if (show || filter == false)
                {
                    display.drawRoundRect(0, 16, 128, 48, 5, WHITE);
                    display.fillRoundRect(1, 17, 126, 10, 2, WHITE);
                    display.setTextColor(BLACK);
                    display.setCursor(40, 18);
                    display.print("TELEMETRY");
                    display.setFont();
                    display.setTextColor(WHITE);
                    display.setCursor(2, 28);
                    display.print(Telemetry[idx].PARM[0]);
                    display.print(":");

                    if (fmod(Telemetry[idx].VAL[0], 1) == 0)
                        display.print(Telemetry[idx].VAL[0], 0);
                    else
                        display.print(Telemetry[idx].VAL[0], 1);
                    display.print(Telemetry[idx].UNIT[0]);
                    display.setCursor(65, 28);
                    display.print(Telemetry[idx].PARM[1]);
                    display.print(":");
                    if (fmod(Telemetry[idx].VAL[1], 1) == 0)
                        display.print(Telemetry[idx].VAL[1], 0);
                    else
                        display.print(Telemetry[idx].VAL[1], 1);
                    display.print(Telemetry[idx].UNIT[1]);
                    display.setCursor(2, 37);
                    display.print(Telemetry[idx].PARM[2]);
                    display.print(":");
                    if (fmod(Telemetry[idx].VAL[2], 1) == 0)
                        display.print(Telemetry[idx].VAL[2], 0);
                    else
                        display.print(Telemetry[idx].VAL[2], 1);
                    display.print(Telemetry[idx].UNIT[2]);
                    display.setCursor(65, 37);
                    display.print(Telemetry[idx].PARM[3]);
                    display.print(":");
                    if (fmod(Telemetry[idx].VAL[3], 1) == 0)
                        display.print(Telemetry[idx].VAL[3], 0);
                    else
                        display.print(Telemetry[idx].VAL[3], 1);
                    display.print(Telemetry[idx].UNIT[3]);
                    display.setCursor(2, 46);
                    display.print(Telemetry[idx].PARM[4]);
                    display.print(":");
                    display.print(Telemetry[idx].VAL[4], 1);
                    display.print(Telemetry[idx].UNIT[4]);

                    display.setCursor(4, 55);
                    display.print("BIT");
                    uint8_t bit = Telemetry[idx].BITS;
                    for (int i = 0; i < 8; i++)
                    {
                        if (bit & 0x80)
                        {
                            display.fillCircle(30 + (i * 12), 58, 3, WHITE);
                        }
                        else
                        {
                            display.drawCircle(30 + (i * 12), 58, 3, WHITE);
                        }
                        bit <<= 1;
                    }
                    // display.print(Telemetry[idx].BITS, BIN);

                    // display.setFont();
                    // display.setCursor(2, 30);
                    // memset(&text[0], 0, sizeof(text));
                    // memcpy(&text[0], aprs.comment, aprs.comment_len);
                    // display.setTextColor(WHITE);
                    // display.print(aprs.comment);
                    display.display();
                }
                return;
            }
            else if (aprs.packettype & T_STATUS)
            {
                display.drawRoundRect(0, 16, 128, 48, 5, WHITE);
                display.fillRoundRect(1, 17, 126, 10, 2, WHITE);
                display.setTextColor(BLACK);
                display.setCursor(48, 18);
                display.print("STATUS");

                display.setFont();
                display.setCursor(2, 30);
                // memset(&text[0], 0, sizeof(text));
                // memcpy(&text[0], aprs.comment, aprs.comment_len);
                display.setTextColor(WHITE);
                display.print(aprs.comment);
                display.display();
                return;
            }
            else if (aprs.packettype & T_QUERY)
            {
                display.drawRoundRect(0, 16, 128, 48, 5, WHITE);
                display.fillRoundRect(1, 17, 126, 10, 2, WHITE);
                display.setTextColor(BLACK);
                display.setCursor(48, 18);
                display.print("?QUERY?");
                // memset(&text[0], 0, sizeof(text));
                // memcpy(&text[0], aprs.comment, aprs.comment_len);
                display.setFont();
                display.setTextColor(WHITE);
                display.setCursor(2, 30);
                display.print(aprs.comment);
                display.display();
                return;
            }
            else if (aprs.packettype & T_MESSAGE)
            {
                if (aprs.msg.is_ack == 1)
                {
                }
                else if (aprs.msg.is_rej == 1)
                {
                }
                else
                {
                    display.drawRoundRect(0, 16, 128, 48, 5, WHITE);
                    display.fillRoundRect(1, 17, 126, 10, 2, WHITE);
                    display.setTextColor(BLACK);
                    display.setCursor(48, 18);
                    display.print("MESSAGE");
                    display.setCursor(108, 18);
                    display.print("{");
                    strncpy(&text[0], aprs.msg.msgid, aprs.msg.msgid_len);
                    int msgid = atoi(text);
                    display.print(msgid, DEC);
                    display.print("}");
                    // memset(&text[0], 0, sizeof(text));
                    // memcpy(&text[0], aprs.comment, aprs.comment_len);
                    display.setFont();
                    display.setTextColor(WHITE);
                    display.setCursor(2, 30);
                    display.print("To: ");
                    strncpy(&text[0], aprs.dstname, aprs.dstname_len);
                    display.print(text);
                    String mycall = config.aprs_mycall + String("-") + String(config.aprs_ssid, DEC);
                    if (strcmp(mycall.c_str(), text) == 0)
                    {
                        display.setCursor(2, 54);
                        display.print("ACK:");
                        display.println(msgid);
                        // String raw = sendIsAckMsg(src_call, msgid);
                        // client.println(raw);
                        // SerialTNC.println("}" + raw);
                        //  if (slot == 0) {
                        //	client.println(raw);
                        //  }
                        //  else {
                        //	SerialTNC.println("}" + raw);
                        //  }
                    }
                    strncpy(&text[0], aprs.msg.body, aprs.msg.body_len);
                    display.setCursor(2, 40);
                    display.print("Msg: ");
                    display.println(text);

                    display.display();
                }
                return;
            }
            display.setFont();
            display.drawFastHLine(0, 16, 128, WHITE);
            display.drawFastVLine(48, 16, 48, WHITE);
            x = 8;

            if (aprs.srcname_len > 0)
            {
                x += 9;
                display.fillRoundRect(51, 16, 77, 9, 2, WHITE);
                display.setTextColor(BLACK);
                display.setCursor(53, x);
                display.print("By " + src_call);
                display.setTextColor(WHITE);
                // x += 9;
            }

            if (aprs.packettype & T_WAVE)
            {
                // Serial.println("WX Display");
                if (aprs.wave_report.flags & O_TEMP)
                {
                    display.setCursor(58, x += 10);
                    display.drawYBitmap(51, x, &Temperature_Symbol[0], 5, 8, WHITE);
                    display.printf("%.2fC", aprs.wave_report.Temp);
                }
                if (aprs.wave_report.flags & O_HS)
                {
                    // display.setCursor(102, x);
                    display.setCursor(58, x += 9);
                    display.printf("Hs:");
                    display.printf("%0.1f M", aprs.wave_report.Hs / 100);
                }
                if (aprs.wave_report.flags & O_TZ)
                {
                    display.setCursor(58, x += 9);
                    display.printf("Tz: ");
                    display.printf("%0.1f S", aprs.wave_report.Tz);
                }
                // if (aprs.wave_report.flags & O_TC)
                // {
                //     display.setCursor(58, x += 9);
                //     display.printf("Tc: ");
                //     display.printf("%0.1fS.", aprs.wave_report.Tc);
                // }
                if (aprs.wave_report.flags & O_BAT)
                {
                    display.setCursor(58, x += 9);
                    display.printf("BAT: ");
                    display.printf("%0.2fV", aprs.wave_report.Bat);
                }
            }
            if (aprs.packettype & T_WX)
            {
                // Serial.println("WX Display");
                if (aprs.wx_report.flags & W_TEMP)
                {
                    display.setCursor(58, x += 10);
                    display.drawYBitmap(51, x, &Temperature_Symbol[0], 5, 8, WHITE);
                    display.printf("%.1fC", aprs.wx_report.temp);
                }
                if (aprs.wx_report.flags & W_HUM)
                {
                    display.setCursor(102, x);
                    display.drawYBitmap(95, x, &Humidity_Symbol[0], 5, 8, WHITE);
                    display.printf("%d%%", aprs.wx_report.humidity);
                }
                if (aprs.wx_report.flags & W_BAR)
                {
                    display.setCursor(58, x += 9);
                    display.drawYBitmap(51, x, &Pressure_Symbol[0], 5, 8, WHITE);
                    display.printf("%.1fhPa", aprs.wx_report.pressure);
                }
                if (aprs.wx_report.flags & W_R24H)
                {
                    // if (aprs.wx_report.rain_1h > 0) {
                    display.setCursor(58, x += 9);
                    display.drawYBitmap(51, x, &Rain_Symbol[0], 5, 8, WHITE);
                    display.printf("%.1fmm.", aprs.wx_report.rain_24h);
                    //}
                }
                if (aprs.wx_report.flags & W_PAR)
                {
                    // if (aprs.wx_report.luminosity > 10) {
                    display.setCursor(51, x += 9);
                    display.printf("%c", 0x0f);
                    display.setCursor(58, x);
                    display.printf("%dW/m", aprs.wx_report.luminosity);
                    if (aprs.wx_report.flags & W_UV)
                    {
                        display.printf(" UV%d", aprs.wx_report.uv);
                    }
                    //}
                }
                if (aprs.wx_report.flags & W_WS)
                {
                    display.setCursor(58, x += 9);
                    display.drawYBitmap(51, x, &Wind_Symbol[0], 5, 8, WHITE);
                    // int dirIdx=map(aprs.wx_report.wind_dir, -180, 180, 0, 8); ((angle+22)/45)%8]
                    int dirIdx = ((aprs.wx_report.wind_dir + 22) / 45) % 8;
                    if (dirIdx > 8)
                        dirIdx = 8;
                    display.printf("%.1fkPh(%s)", aprs.wx_report.wind_speed, directions[dirIdx]);
                }
                // Serial.printf("%.1fkPh(%d)", aprs.wx_report.wind_speed, aprs.wx_report.wind_dir);
                if (aprs.flags & F_HASPOS)
                {
                    // Serial.println("POS Display");
                    double lat, lon;
                    if ((config.mygps == true) && gps.location.isValid())
                    {
                        lat = gps.location.lat();
                        lon = gps.location.lng();
                    }
                    else
                    {
                        lat = config.gps_lat;
                        lon = config.gps_lon;
                    }
                    double dtmp = aprsParse.direction(lon, lat, aprs.lng, aprs.lat);
                    double dist = aprsParse.distance(lon, lat, aprs.lng, aprs.lat);
                    if (config.h_up == true)
                    {
                        // double course = gps.course.deg();
                        double course = SB_HEADING;
                        if (dtmp >= course)
                        {
                            dtmp -= course;
                        }
                        else
                        {
                            double diff = dtmp - course;
                            dtmp = diff + 360.0F;
                        }
                        compass_label(25, 37, 15, course, WHITE);
                        display.setCursor(0, 17);
                        display.printf("H");
                    }
                    else
                    {
                        compass_label(25, 37, 15, 0.0F, WHITE);
                    }
                    // compass_label(25, 37, 15, 0.0F, WHITE);
                    compass_arrow(25, 37, 12, dtmp, WHITE);
                    display.drawFastHLine(1, 63, 45, WHITE);
                    display.drawFastVLine(1, 58, 5, WHITE);
                    display.drawFastVLine(46, 58, 5, WHITE);
                    display.setCursor(4, 55);
                    if (dist > 999)
                        display.printf("%.fKm", dist);
                    else
                        display.printf("%.1fKm", dist);
                }
                else
                {
                    display.setCursor(20, 30);
                    display.printf("NO\nPOSITION");
                }
            }
            else if (aprs.flags & F_HASPOS)
            {
                // display.setCursor(50, x += 10);
                // display.printf("LAT %.5f\n", aprs.lat);
                // display.setCursor(51, x+=9);
                // display.printf("LNG %.4f\n", aprs.lng);
                String str;
                int l = 0;
                display.setCursor(50, x += 10);
                display.print("LAT:");
                str = String(aprs.lat, 5);
                l = str.length() * 6;
                display.setCursor(128 - l, x);
                display.print(str);

                display.setCursor(50, x += 9);
                display.print("LON:");
                str = String(aprs.lng, 5);
                l = str.length() * 6;
                display.setCursor(128 - l, x);
                display.print(str);

                double lat, lon;
                if ((config.mygps == true) && gps.location.isValid())
                {
                    lat = gps.location.lat();
                    lon = gps.location.lng();
                }
                else
                {
                    lat = config.gps_lat;
                    lon = config.gps_lon;
                }
                double dtmp = aprsParse.direction(lon, lat, aprs.lng, aprs.lat);
                double dist = aprsParse.distance(lon, lat, aprs.lng, aprs.lat);
                if (config.h_up == true)
                {
                    // double course = gps.course.deg();
                    double course = SB_HEADING;
                    if (dtmp >= course)
                    {
                        dtmp -= course;
                    }
                    else
                    {
                        double diff = dtmp - course;
                        dtmp = diff + 360.0F;
                    }
                    compass_label(25, 37, 15, course, WHITE);
                    display.setCursor(0, 17);
                    display.printf("H");
                }
                else
                {
                    compass_label(25, 37, 15, 0.0F, WHITE);
                }
                compass_arrow(25, 37, 12, dtmp, WHITE);
                display.drawFastHLine(1, 55, 45, WHITE);
                display.drawFastVLine(1, 55, 5, WHITE);
                display.drawFastVLine(46, 55, 5, WHITE);
                display.setCursor(4, 57);
                if (dist > 999)
                    display.printf("%.fKm", dist);
                else
                    display.printf("%.1fKm", dist);
                if (aprs.flags & F_CSRSPD)
                {
                    display.setCursor(51, x += 9);
                    // display.printf("SPD %d/", aprs.course);
                    // display.setCursor(50, x += 9);
                    display.printf("SPD %.1fkPh\n", aprs.speed);
                    int dirIdx = ((aprs.course + 22) / 45) % 8;
                    if (dirIdx > 8)
                        dirIdx = 8;
                    display.setCursor(51, x += 9);
                    display.printf("CSD %d(%s)", aprs.course, directions[dirIdx]);
                }
                if (aprs.flags & F_ALT)
                {
                    display.setCursor(51, x += 9);
                    display.printf("ALT %.1fM\n", aprs.altitude);
                }
                if (aprs.flags & F_PHG)
                {
                    int power, height, gain;
                    unsigned char tmp;
                    power = (int)aprs.phg[0] - 0x30;
                    power *= power;
                    height = (int)aprs.phg[1] - 0x30;
                    height = 10 << (height + 1);
                    height = height / 3.2808;
                    gain = (int)aprs.phg[2] - 0x30;
                    display.setCursor(51, x += 9);
                    display.printf("PHG %dM.\n", height);
                    display.setCursor(51, x += 9);
                    display.printf("PWR %dWatt\n", power);
                    display.setCursor(51, x += 9);
                    display.printf("ANT %ddBi\n", gain);
                }
                if (aprs.flags & F_RNG)
                {
                    display.setCursor(51, x += 9);
                    display.printf("RNG %dKm\n", aprs.radio_range);
                }
                /*if (aprs.comment_len > 0) {
                    display.setCursor(0, 56);
                    display.print(aprs.comment);
                }*/
            }
            display.display();
        }
    }
}

String cut_string(String input, String header)
{
    if (input.indexOf(header) != -1) // ตรวจสอบว่าใน input มีข้อความเหมือนใน header หรือไม่
    {
        int num_get = input.indexOf(header); // หาตำแหน่งของข้อความ get_string ใน input
        if (num_get != -1)                   // ตรวจสอบว่าตำแหน่งที่ได้ไม่ใช่ -1 (ไม่มีข้อความ get_string ใน input)
        {
            int start_val = input.indexOf(">", num_get) + 1; // หาตำแหน่งแรกของ “
            int stop_val = input.indexOf(",", start_val);    // หาตำแหน่งสุดท้ายของ “
            return (input.substring(start_val, stop_val));   // ตัดเอาข้อความระหว่า “แรก และ ”สุดท้าย
        }
        else
        {
            return ("NULL"); // Return ข้อความ NULL เมื่อไม่ตรงเงื่อนไข
        }
    }

    return ("NULL"); // Return ข้อความ NULL เมื่อไม่ตรงเงื่อนไข
}