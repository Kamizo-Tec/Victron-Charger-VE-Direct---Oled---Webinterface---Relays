

#include <Arduino.h>
#include "VeDirectFrameHandler.h"

#define MODULE "VE.Frame" // Victron seems to use this to find out where logging messages were generated

// The name of the record that contains the checksum.
static constexpr char checksumTagName[] = "CHECKSUM";


// neu 

VeDirectFrameHandler::VeDirectFrameHandler()
    : frameIndex(0),
      veEnd(0),
      veError(1),
      mState(IDLE),
      mChecksum(0),
      mTextPointer(nullptr)
{
    // Alle Puffer sauber auf 0 setzen
    memset(mName, 0, sizeof(mName));
    memset(mValue, 0, sizeof(mValue));
    memset(tempName, 0, sizeof(tempName));
    memset(tempValue, 0, sizeof(tempValue));
    memset(veName, 0, sizeof(veName));
    memset(veValue, 0, sizeof(veValue));
}

/*
 *	rxData
 *  This function is called by the application which passes a byte of serial data
 *  It is unchanged from Victron's example code
 */

void VeDirectFrameHandler::rxData(uint8_t inbyte)
{
    static uint8_t originalByte = 0;
    static bool lastWasNewline = false;

    // HEX-Frames erkennen
    if ((inbyte == ':') && (mState != CHECKSUM)) {
        mState = RECORD_HEX;
      //  Serial.println("🔄 HEX-Frame erkannt");
    }

    // Originalbyte merken
    originalByte = inbyte;

    // Checksumme berechnen (außer bei HEX und CHECKSUM)
    if (mState != RECORD_HEX && mState != CHECKSUM) {
        mChecksum += originalByte;
    }

    // Für Parsing Großbuchstaben verwenden
    uint8_t byteUpper = toupper(inbyte);

    // Debug: Byte anzeigen
  //  Serial.printf("📥 Byte: '%c' (0x%02X), State: %d, Checksum: %u\n", inbyte, inbyte, mState, mChecksum & 0xFF);

    switch (mState)
    {
    case IDLE:
        if (byteUpper == '\n') {
            if (lastWasNewline && frameIndex > 0) {
            //    Serial.println("🔚 Doppelte \\n erkannt → Frame-Ende");
                frameEndEvent(true); // Fallback: Frame ohne CHECKSUM
                mChecksum = 0;
                frameIndex = 0;
            }
            lastWasNewline = true;
            mState = RECORD_BEGIN;
        } else {
            lastWasNewline = false;
        }
        break;

    case RECORD_BEGIN:
        mTextPointer = mName;
        if ((size_t)(mTextPointer - mName) < sizeof(mName) - 1) {
            *mTextPointer++ = byteUpper;
            *mTextPointer = '\0';
        }
        mState = RECORD_NAME;
        break;

    case RECORD_NAME:
        if (byteUpper == '\t') {
            *mTextPointer = '\0';
         //   Serial.printf("📝 Name erkannt: %s\n", mName);
            if (strcmp(mName, checksumTagName) == 0) {
                mState = CHECKSUM;
              //  Serial.println("🔍 CHECKSUM-Feld erkannt");
                break;
            }
            mTextPointer = mValue;
            *mTextPointer = '\0';
            mState = RECORD_VALUE;
        } else if ((size_t)(mTextPointer - mName) < sizeof(mName) - 1) {
            *mTextPointer++ = byteUpper;
            *mTextPointer = '\0';
        }
        break;

case RECORD_VALUE:
    if (inbyte == '\n') {   // Frame-Ende
        *mTextPointer = '\0';
      //  Serial.printf(" Wert erkannt: %s = %s\n", mName, mValue);
        textRxEvent(mName, mValue);
        mState = RECORD_BEGIN;
    } 
    else if (inbyte == '\r') {
        // CR ignorieren, aber String terminieren
        *mTextPointer = '\0';
    }
    else if (inbyte == '\t') {
        // Sicherheit: Falls doch ein Tab mitten drin kommt → abschließen
        *mTextPointer = '\0';
      //  Serial.printf(" Wert erkannt: %s = %s\n", mName, mValue);
        textRxEvent(mName, mValue);
        mTextPointer = mName;
        *mTextPointer = '\0';
        mState = RECORD_NAME;
    }
    else if ((size_t)(mTextPointer - mValue) < sizeof(mValue) - 1) {
        *mTextPointer++ = inbyte;   // WICHTIG: hier nicht toupper()
        *mTextPointer = '\0';
    }
    break;


    case CHECKSUM:
    {
        bool valid = (mChecksum & 0xFF) == 0;
      //  Serial.printf("🔍 Checksumme geprüft: %u → %s\n", mChecksum & 0xFF, valid ? "gültig" : "ungültig");

        if (!valid) {
            veError++;
          //  logE((char *)MODULE, (char *)"[CHECKSUM] Invalid frame");
         //   Serial.println("checksum faied Ungültiger VE.Direct-Frame ignoriert");

           // frameIndex = 0;

frameEndEvent(false);  //falschen checksum trotzdem verarbeiten!

        } else {
            Serial.println("found correct VE.Direct-Frame ");
        }

        mChecksum = 0;
        mState = IDLE;
        frameEndEvent(valid);
        break;
    }

    case RECORD_HEX:
        if (hexRxEvent(byteUpper)) {
         //   Serial.println("📤 HEX-Frame verarbeitet");
            mChecksum = 0;
            mState = IDLE;
        }
        break;
    }
}


/* ----------------------------------------------------------
 * textRxEvent
 * This function is called every time a new name/value is successfully parsed.  
 * It writes the values to the temporary buffer.
*/

void VeDirectFrameHandler::textRxEvent(char *name, char *value)
{
    if (frameIndex >= frameLen) return; // zu viele Einträge → ignorieren

    // sicher kopieren: max. nameLen-1 bzw. valueLen-1 Zeichen
    strncpy(tempName[frameIndex], name, nameLen - 1);
    tempName[frameIndex][nameLen - 1] = '\0';

    strncpy(tempValue[frameIndex], value, valueLen - 1);
    tempValue[frameIndex][valueLen - 1] = '\0';

// 🐞 Debug-Ausgabe array checksum anzeigen
 //   Serial.printf(" [%02d] %s = %s\n", frameIndex, tempName[frameIndex], tempValue[frameIndex]);
//Serial.printf(" [%02d] %s = %s\n", frameIndex, name, value);


    frameIndex++;
}


//**********new  *********************************

void VeDirectFrameHandler::frameEndEvent(bool valid)
{
    if (!valid) {
        veError++;
      //  Serial.println("⚠️ Ungültiger VE.Direct-Frame ignoriert aber trotzdem verarbeitet");
        // Frame trotzdem verarbeiten
    } else {
        veError = 0;
    }

    for (int i = 0; i < frameIndex; i++) {
        bool nameExists = false;

        for (int j = 0; j < veEnd; j++) {
            if (strcmp(tempName[i], veName[j]) == 0) {
                strncpy(veValue[j], tempValue[i], sizeof(veValue[j]) - 1);
                veValue[j][sizeof(veValue[j]) - 1] = '\0';
                nameExists = true;
                break;
            }
        }

        if (!nameExists && veEnd < buffLen) {
            strncpy(veName[veEnd], tempName[i], sizeof(veName[veEnd]) - 1);
            veName[veEnd][sizeof(veName[veEnd]) - 1] = '\0';

            strncpy(veValue[veEnd], tempValue[i], sizeof(veValue[veEnd]) - 1);
            veValue[veEnd][sizeof(veValue[veEnd]) - 1] = '\0';

            veEnd++;
        }
    }

    frameIndex = 0;

    // ✅ Callback immer auslösen – auch bei ungültigen Frames
    if (requestCallback) {
        requestCallback();
    }
}

//----------------------------------------------------

/*
 *	logE
 *  This function included for continuity and possible future use.
 */
void VeDirectFrameHandler::logE(char *module, char *error)
{
	Serial.print("MODULE: ");
	Serial.println(module);
	Serial.print("ERROR: ");
	Serial.println(error);
	return;
}

/*
 *	hexRxEvent
 *  This function included for continuity and possible future use.
 */
bool VeDirectFrameHandler::hexRxEvent(uint8_t inbyte)
{
	return true; // stubbed out for future
}

void VeDirectFrameHandler::callback(std::function<void()> func) // callback function when finnish request
{
	requestCallback = func;
}

/*
// Geratenamen aus hex

inline const char* getVeDirectDeviceName(const char* deviceId) {
    for (size_t i = 0; i < VeDirectDeviceListCount; i++) {
        char id[8];
        strcpy_P(id, (char*)pgm_read_word(&(VeDirectDeviceList[i][0])));
        if (strcasecmp(id, deviceId) == 0) {
            return (const char*)pgm_read_word(&(VeDirectDeviceList[i][1]));
        }
    }
    return deviceId; // fallback
}
*/