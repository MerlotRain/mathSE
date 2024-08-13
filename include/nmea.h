/*****************************************************************************/
/*  Math Spatial Engine - Open source 2D geometry algorithm library          */
/*                                                                           */
/*  Copyright (C) 2013-2024 Merlot.Rain                                      */
/*                                                                           */
/*  This library is free software, licensed under the terms of the GNU       */
/*  General Public License as published by the Free Software Foundation,     */
/*  either version 3 of the License, or (at your option) any later version.  */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*****************************************************************************/

#ifndef NMEA_H
#define NMEA_H

#ifdef __cplusplus
extern "C" {
#endif

#define NMEA_MAXSAT    (12)
#define NMEA_SATINPACK (4)
#define NMEA_NSATPACKS (NMEA_MAXSAT / NMEA_SATINPACK)

/**
 * Information about satellite
 * \see nmeaSATINFO
 * \see nmeaGPGSV
 */
struct nmeaSATELLITE {
    int id;      ///< Satellite PRN number
    int in_use;  ///< Used in position fix
    int elv;     ///< Elevation in degrees, 90 maximum
    int azimuth; ///< Azimuth, degrees from true north, 000 to 359
    int sig;     ///< Signal, 00-99 dB
};

struct nmeaTIME {
    int year; ///< Years since 1900
    int mon;  ///< Months since January - [0,11]
    int day;  ///< Day of the month - [1,31]
    int hour; ///< Hours since midnight - [0,23]
    int min;  ///< Minutes after the hour - [0,59]
    int sec;  ///< Seconds after the minute - [0,59]
    int msec; ///< Thousandths part of second - [0,999]
};

/**
 * NMEA packets type which parsed and generated by library
 */
enum nmeaPACKTYPE {
    GPNON = 0x0000, ///< Unknown packet type.
    GPGGA = 0x0001, ///< GGA - Essential fix data which provide 3D location and
                    ///< accuracy data.
    GPGSA = 0x0002, ///< GSA - GPS receiver operating mode, SVs used for
                    ///< navigation, and DOP values.
    GPGSV = 0x0004, ///< GSV - Number of SVs in view, PRN numbers, elevation,
                    ///< azimuth & SNR values.
    GPRMC = 0x0008, ///< RMC - Recommended Minimum Specific GPS/TRANSIT Data.
    GPVTG = 0x0010, ///< VTG - Actual track made good and speed over ground.
    GPGST = 0x0012, ///< GST - GPS Pseudorange Noise Statistics
    HCHDG = 0x0020, ///< HDG - Heading, Deviation and Variation
    HCHDT = 0x0100, ///< HDT - Heading reference to true north
};

/**
 * GGA packet information structure (Global Positioning System Fix Data)
 */
struct nmeaGPGGA {
    struct nmeaTIME utc; ///< UTC of position (just time)
    double lat;          ///< Latitude in NDEG - [degree][min].[sec/60]
    char ns;             ///< [N]orth or [S]outh
    double lon;          ///< Longitude in NDEG - [degree][min].[sec/60]
    char ew;             ///< [E]ast or [W]est
    int sig; ///< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential,
             ///< 3 = Sensitive)
    int satinuse;   ///< Number of satellites in use (not those in view)
    double HDOP;    ///< Horizontal dilution of precision
    double elv;     ///< Antenna altitude above/below mean sea level (geoid)
    char elv_units; ///< [M]eters (Antenna height unit)
    double diff; ///< Geoidal separation (Diff. between WGS-84 earth ellipsoid
                 ///< and mean sea level. '-' = geoid is below WGS-84 ellipsoid)
    char diff_units; ///< [M]eters (Units of geoidal separation)
    double dgps_age; ///< Time in seconds since last DGPS update
    int dgps_sid;    ///< DGPS station ID number
};

/**
 * GST packet information structure (GPS Pseudorange Noise Statistics)
 */
struct nmeaGPGST {
    struct nmeaTIME utc; ///< UTC of position fix
    double rms_pr; ///< RMS value of the pseudorange residuals; Includes carrier
                   ///< phase residuals during periods of RTK (float) and RTK
                   ///< (fixed) processing
    double
        err_major; ///< Error ellipse semi-major axis 1 sigma error, in meters
    double
        err_minor;  ///< Error ellipse semi-minor axis 1 sigma error, in meters
    double err_ori; ///< Error ellipse orientation, degrees from true north
    double sig_lat; ///< Latitude 1 sigma error, in meters
    double sig_lon; ///< Longitude 1 sigma error, in meters
    double sig_alt; ///< Height 1 sigma error, in meters
};

/**
 * GSA packet information structure (Satellite status)
 */
struct nmeaGPGSA {
    char fix_mode; ///< Mode (M = Manual, forced to operate in 2D or 3D; A =
                   ///< Automatic, 3D/2D)
    int fix_type; ///< Type, used for navigation (1 = Fix not available; 2 = 2D;
                  ///< 3 = 3D)
    int sat_prn[NMEA_MAXSAT]; ///< PRNs of satellites used in position fix (null
                              ///< for unused fields)
    double PDOP;              ///< Dilution of precision
    double HDOP;              ///< Horizontal dilution of precision
    double VDOP;              ///< Vertical dilution of precision
    char pack_type;           ///< P=GPS, N=generic, L=GLONASS

} nmeaGPGSA;

/**
 * GSV packet information structure (Satellites in view)
 */
struct nmeaGPGSV {
    int pack_count; ///< Total number of messages of this type in this cycle
    int pack_index; ///< Message number
    int sat_count;  ///< Total number of satellites in view
    char pack_type; ///< P=GPS - S=SBas - N=generic - L=GLONAS - A=GALILEO -
                    ///< B=BEIDOU - Q=QZSS
    struct nmeaSATELLITE sat_data[NMEA_SATINPACK];
};

/**
 * RMC packet information structure (Recommended Minimum sentence C)
 */
struct nmeaGPRMC {
    struct nmeaTIME utc; ///< UTC of position
    char status;         ///< Status (A = active or V = void)
    double lat;          ///< Latitude in NDEG - [degree][min].[sec/60]
    char ns;             ///< [N]orth or [S]outh
    double lon;          ///< Longitude in NDEG - [degree][min].[sec/60]
    char ew;             ///< [E]ast or [W]est
    double speed;        ///< Speed over the ground in knots
    double direction;    ///< Track angle in degrees True
    double declination; ///< Magnetic variation degrees (Easterly var. subtracts
                        ///< from true course)
    char declin_ew;     ///< [E]ast or [W]est
    char mode;          ///< Mode indicator of fix type (A = autonomous, D =
               ///< differential, E = estimated, N = not valid, S = simulator)
    char navstatus; ///< NMEA v4.1 - Navigation Status type (S = Safe, C =
                    ///< Caution, U = Unsafe, V = Navigational status not valid)
};

/**
 * VTG packet information structure (Track made good and ground speed)
 */
struct nmeaGPVTG {
    double dir; ///< True track made good (degrees)
    char dir_t; ///< Fixed text 'T' indicates that track made good is relative
                ///< to true north
    double dec; ///< Magnetic track made good
    char dec_m; ///< Fixed text 'M'
    double spn; ///< Ground speed, knots
    char spn_n; ///< Fixed text 'N' indicates that speed over ground is in knots
    double spk; ///< Ground speed, kilometers per hour
    char spk_k; ///< Fixed text 'K' indicates that speed over ground is in
                ///< kilometers/hour
};

/**
 * HDT packet information structure (Heading from True North)
 */
struct nmeaGPHDT {
    double heading; ///< Heading in degrees
    char t_flag; ///< Fixed text 'T' indicates that heading is relative to true
                 ///< north
};

/**
 * HCHDG packet information structure (magnetic heading)
 */
struct nmeaHCHDG {
    double mag_heading;   ///< Magnetic sensor heading (degrees)
    double mag_deviation; ///< Magnetic deviation (degrees)
    char ew_deviation;    ///< [E]ast or [W]est
    double mag_variation; ///< Magnetic variation (degrees)
    char ew_variation;    ///< [E]ast or [W]est
};

/**
 * HDT packet information structure (Heading, )
 */
struct nmeaHCHDT {
    double direction; ///< Heading respect to true north (degrees)
    char t_flag;      ///< Static text [T]
};

int nmea_parse_GPGGA(const char *buf, int len, struct nmeaGPGGA *pack);
int nmea_parse_GPGSA(const char *buf, int len, struct nmeaGPGSA *pack);
int nmea_parse_GPGSV(const char *buf, int len, struct nmeaGPGSV *pack);
int nmea_parse_GPRMC(const char *buf, int len, struct nmeaGPRMC *pack);
int nmea_parse_GPVTG(const char *buf, int len, struct nmeaGPVTG *pack);
int nmea_parse_HCHDG(const char *buf, int len, struct nmeaHCHDG *pack);
int nmea_parse_HCHDT(const char *buf, int len, struct nmeaHCHDT *pack);
int nmea_parse_GPGST(const char *buf, int len, struct nmeaGPGST *pack);
int nmea_parse_GPHDT(const char *buf, int len, struct nmeaGPHDT *pack);

#ifdef __cplusplus
}
#endif

#endif
