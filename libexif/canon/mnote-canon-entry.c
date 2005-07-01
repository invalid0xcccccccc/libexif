/* mnote-canon-entry.c
 *
 * Copyright � 2002 Lutz M�ller <lutz@users.sourceforge.net>
 * Copyright � 2003 Matthieu Castet <mat-c@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "mnote-canon-entry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libexif/exif-format.h>
#include <libexif/exif-utils.h>
#include <libexif/i18n.h>

/* #define DEBUG */

#undef  MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#define CF(format,target,v,maxlen)                              \
{                                                               \
        if (format != target) {                                 \
                snprintf (v, maxlen,                            \
                        _("Invalid format '%s', "               \
                        "expected '%s'."),                      \
                        exif_format_get_name (format),          \
                        exif_format_get_name (target));         \
                break;                                          \
        }                                                       \
}

#define CC(number,target,v,maxlen)                                      \
{                                                                       \
        if (number != target) {                                         \
                snprintf (v, maxlen,                                    \
                        _("Invalid number of components (%i, "          \
                        "expected %i)."), (int) number, (int) target);  \
                break;                                                  \
        }                                                               \
}
#define CC2(number,t1,t2,v,maxlen)                                      \
{                                                                       \
	if ((number != t1) && (number != t2)) {                         \
		snprintf (v, maxlen,                                    \
			_("Invalid number of components (%i, "          \
			"expected %i or %i)."), (int) number,		\
			(int) t1, (int) t2);  				\
		break;                                                  \
	}                                                               \
}

char *
mnote_canon_entry_get_value (const MnoteCanonEntry *entry, char *val, unsigned int maxlen)
{
    char buf[128];
    ExifLong vl;
    ExifShort vs, n;
    int i;
    unsigned char *data = entry->data;


#define UNDEFINED 0xFF
    
static struct  {
  int tag;
  int subtag;
  char* value;
} entries [] = {
  {1,1,N_("Macro")},
  {1,2,N_("Normal")},
  {1,UNDEFINED,N_("%i???")},
  {4,0,N_("Flash not fired")},
  {4,1,N_("auto")},
  {4,2,N_("on")},
  {4,3,N_("red eyes reduction")},
  {4,4,N_("slow synchro")},
  {4,5,N_("auto + red yes reduction")},
  {4,6,N_("on + red eyes reduction")},
  {4,16,N_("external")},
  {4,UNDEFINED,N_("%i???")},
  {5,0,N_("single or timer")},
  {5,1,N_("continuous")},
  {5,UNDEFINED,N_("%i???")},
  {7,0,N_("One-Shot")},
  {7,1,N_("AI Servo")},
  {7,2,N_("AI Focus")},
  {7,3,N_("MF")},
  {7,4,N_("Single")},
  {7,5,N_("Continuous")},
  {7,6,N_("MF")},
  {7,UNDEFINED,N_("%i???")},
  {10,0,N_("Large")},
  {10,1,N_("Medium")},
  {10,2,N_("Small")},
  {10,UNDEFINED,N_("%i???")},
  {11,0,N_("Full Auto")},
  {11,1,N_("Manual")},
  {11,2,N_("Landscape")},
  {11,3,N_("Fast Shutter")},
  {11,4,N_("Slow Shutter")},
  {11,5,N_("Night")},
  {11,6,N_("Black & White")},
  {11,7,N_("Sepia")},
  {11,8,N_("Portrait")},
  {11,9,N_("Sports")},
  {11,10,N_("Macro / Close-Up")},
  {11,11,N_("Pan Focus")},
  {11,UNDEFINED,N_("%i???")},
  {13,0xffff,N_("Low")},
  {13,0x0000,N_("Normal")},
  {13,0x0001,N_("High")},
  {13,UNDEFINED,N_("%i???")},
  {14,0xffff,N_("Low")},
  {14,0x0000,N_("Normal")},
  {14,0x0001,N_("High")},
  {14,UNDEFINED,N_("%i???")},
  {15,0xffff,N_("Low")},
  {15,0x0000,N_("Normal")},
  {15,0x0001,N_("High")},
  {15,UNDEFINED,N_("%i???")},
  {16,15,N_("auto")},
  {16,16,N_("50")},
  {16,17,N_("100")},
  {16,18,N_("200")},
  {16,19,N_("400")},
  {16,UNDEFINED,N_("%i???")},
  {17,3,N_("Evaluative")},
  {17,4,N_("Partial")},
  {17,5,N_("Center-weighted")},
  {17,UNDEFINED,N_("%i???")},
  {19,0x3000,N_("none (MF)")},
  {19,0x3001,N_("auto-selected")},
  {19,0x3002,N_("right")},
  {19,0x3003,N_("center")},
  {19,0x3004,N_("left")},
  {19,UNDEFINED,N_("%i???")},
  {20,0,N_("Easy shooting")},
  {20,1,N_("Program")},
  {20,2,N_("Tv-priority")},
  {20,3,N_("Av-priority")},
  {20,4,N_("Manual")},
  {20,5,N_("A-DEP")},
  {20,UNDEFINED,N_("%i???")},
  {32,0,N_("Single")},
  {32,1,N_("Continuous")},
  {32,UNDEFINED,N_("%i???")},
  
  {0,0,NULL}
};

static struct {
  int index;
  char* value;
}  headings[] =  {
  {1,N_("Macro mode")},
  {4,N_(" / Flash mode : ")},
  {5,N_(" / Continuous drive mode : ")},
  {7,N_(" / Focus mode : ")},
  {10,N_(" / Image size : ")},
  {11,N_(" / Easy shooting mode : ")},
  {13,N_(" / Contrast : ")},
  {14,N_(" / Saturation : ")},
  {15,N_(" / Sharpness : ")},
  {16,N_(" / ISO : ")},
  {17,N_(" / Metering mode : ")},
  {19,N_(" / AF point selected : ")},
  {20,N_(" / Exposure mode : ")},
  {32,N_(" / Focus mode2 : ")},

  {0,NULL}

};


    if (!entry) return NULL;

    memset (val, 0, maxlen);
    maxlen--;

    switch (entry->tag) {
	case MNOTE_CANON_TAG_SETTINGS_1:
		CF (entry->format, EXIF_FORMAT_SHORT, val, maxlen);
		n = exif_get_short (data, entry->order) / 2;
		data += 2;
		CC (entry->components, n, val, maxlen);
		for (i = 1; i < n; i++) {
		    vs = exif_get_short (data, entry->order);
		    data += 2;
		    switch (i) {
		    case 2:
			    if (vs) {
				    snprintf (buf, sizeof (buf),
					    _(" / Self Timer : %i (ms)"), vs*100);
				    strncat (val, buf, maxlen - strlen(val));
			    }
			    break;
		    case 23:
			    snprintf (buf, sizeof (buf), _(" / long focal length of lens (in focal units) : %u"), vs);
			    strncat (val, buf, maxlen - strlen(val));
			    break;
		    case 24:
			    snprintf (buf, sizeof (buf), _(" / short focal length of lens (in focal units) : %u"), vs);
			    strncat (val, buf, maxlen - strlen(val));
			    break;
		    case 25:
			    snprintf (buf, sizeof (buf), _(" / focal units per mm : %u"), vs);
			    strncat (val, buf, maxlen - strlen(val));
			    break;
		    case 29:
			    strncat (val, _(" / Flash details : "), maxlen - strlen(val));
			    if ((vs>>14)&1)
				    strncat (val, _("External E-TTL"), maxlen - strlen(val));
			    if ((vs>>13)&1)
				    strncat (val, _("Internal flash"), maxlen - strlen(val));
			    if ((vs>>11)&1)
				    strncat (val, _("FP sync used"), maxlen - strlen(val));
			    if ((vs>>4)&1)
				    strncat (val, _("FP sync enabled"), maxlen - strlen(val));
#ifdef DEBUG
			    printf ("Value29=0x%08x\n", vs);
#endif
			    break;
		    default:
			{
			int index;
			int  found = 0;
			for (index=0; entries[index].tag <= i && entries[index].value && vs && !found; i++) {
				if (entries[index].tag == i) {
				  	int a=0;
					for (a=0; headings[a].index != i; a++);
				 	strncat (val, headings[a].value, maxlen - strlen(val)); 

					if (entries[index].subtag == vs) {
						strncat(val,entries[index].value, maxlen - strlen(val));
						found = 1;
					}
					if (entries[index].subtag == UNDEFINED) {
						snprintf (buf, sizeof (buf), entries[index].value, vs);
						strncat (val, buf, maxlen - strlen(val)); 
						found = 1;

					}
				}
			}
			if (!found)
			    printf ("Value%d=%d\n", i, vs);
		    	}
		    }
		} // for

                break;

	case MNOTE_CANON_TAG_SETTINGS_2:
		CF (entry->format, EXIF_FORMAT_SHORT, val, maxlen);
		n = exif_get_short (data, entry->order)/2;
		data += 2;
		CC (entry->components, n, val, maxlen);
#ifdef DEBUG
		printf ("Setting2 size %d %d\n",n,entry->size);
#endif
		for (i=1;i<n;i++)
		{
			vs = exif_get_short (data, entry->order);
			data+=2;
			switch(i) {
			case 7:
				strncpy (val, _("White balance : "), maxlen - strlen(val));
				switch (vs) {
				case 0:
					strncat (val, _("Auto"), maxlen - strlen(val));
					break;
				case 1:
					strncat (val, _("Sunny"), maxlen - strlen(val));
					break;
				case 2:
					strncat (val, _("Cloudy"), maxlen - strlen(val));
					break;
				case 3:
					strncat (val, _("Tungsten"), maxlen - strlen(val));
					break;
				case 4:
					strncat (val, _("Flourescent"), maxlen - strlen(val));
					break;
				case 5:
					strncat (val, _("Flash"), maxlen - strlen(val));
					break;
				case 6:
					strncat (val, _("Custom"), maxlen - strlen(val));
					break;
				default:
					snprintf (buf, sizeof (buf), _("%i???"), vs);
					strncat (val, buf, maxlen - strlen(val));
				}
				break;
			case 9:
				snprintf (buf, sizeof (buf), _(" / Sequence number : %u"), vs);
				strncat (val, buf, maxlen - strlen(val));
				break;
			case 14:
				if (vs>>12)
				{
					strncat (val, _(" / AF point used : "), maxlen - strlen(val));
					if (vs&1)
						strncat (val, _("Right"), maxlen - strlen(val));
					if ((vs>>1)&1)
						strncat (val, _("Center"), maxlen - strlen(val));
					if ((vs>>2)&1)
						strncat (val, _("Left"), maxlen - strlen(val));
					snprintf (buf, sizeof (buf), _(" (%u available focus point)"), vs>>12);
					strncat (val, buf, maxlen - strlen(val));
				}
#ifdef DEBUG
					printf ("0x%08x\n", vs);
#endif
				break;
			case 15:
				snprintf (buf, sizeof (buf), _(" / Flash bias : %.2f EV"), vs/32.0);
				strncat (val, buf, maxlen - strlen(val));

				break;
			case 19:
				snprintf (buf, sizeof (buf), _(" / Subject Distance (mm) : %u"), vs);
				strncat (val, buf, maxlen - strlen(val));
				break;
#ifdef DEBUG
			default:
				printf ("Value%d=%d\n", i, vs);
#endif
			}
		}

		break;

	case MNOTE_CANON_TAG_IMAGE_TYPE:
	case MNOTE_CANON_TAG_OWNER:
		CF (entry->format, EXIF_FORMAT_ASCII, val, maxlen);
		CC (entry->components, 32, val, maxlen);
		strncpy (val, (char *)data, MIN (entry->size, maxlen));
		break;

	case MNOTE_CANON_TAG_FIRMWARE:
		CF (entry->format, EXIF_FORMAT_ASCII, val, maxlen);
		CC2 (entry->components, 24, 32, val, maxlen);
		strncpy (val, (char *)data, MIN (entry->size, maxlen));
		break;

	case MNOTE_CANON_TAG_IMAGE_NUMBER:
		CF (entry->format, EXIF_FORMAT_LONG, val, maxlen);
		CC (entry->components, 1, val, maxlen);
		vl = exif_get_long (data, entry->order);
		snprintf (val, maxlen, "%03lu-%04lu",
				(unsigned long) vl/10000,
				(unsigned long) vl%10000);
		break;

	case MNOTE_CANON_TAG_SERIAL_NUMBER:
		CF (entry->format, EXIF_FORMAT_LONG, val, maxlen);
		CC (entry->components, 1, val, maxlen);
		vl = exif_get_long (data, entry->order);
		snprintf (val, maxlen, "%04X-%05d", (int)vl>>16,(int)vl&0xffff);
		break;

	case MNOTE_CANON_TAG_CUSTOM_FUNCS:
		CF (entry->format, EXIF_FORMAT_SHORT, val, maxlen);
		n = exif_get_short (data, entry->order)/2;
		data+=2;
		CC (entry->components, n, val, maxlen);
#ifdef DEBUG
		printf ("Custom Function size %d %d\n",n,entry->size);
#endif
		for (i=1;i<n;i++)
		{
			vs = exif_get_short (data, entry->order);
			data += 2;
			snprintf (buf, sizeof(buf), _("C.F%d : %u"), i, vs);
			strncat (val, buf, maxlen - strlen(val));
		}
		break;

	default:
#ifdef DEBUG
		if (entry->format == EXIF_FORMAT_SHORT)
		for(i=0;i<entry->components;i++) {
			vs = exif_get_short (data, entry->order);
			data+=2;
			printf ("Value%d=%d\n", i, vs);
		}
		else if (entry->format == EXIF_FORMAT_LONG)
		for(i=0;i<entry->components;i++) {
			vl = exif_get_long (data, entry->order);
			data+=4;
			printf ("Value%d=%d\n", i, vs);
		}
		else if (entry->format == EXIF_FORMAT_ASCII)
		    strncpy (val, data, MIN (entry->size, maxlen));
#endif
		break;
        }
        return val;
}
