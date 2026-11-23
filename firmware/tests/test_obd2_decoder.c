#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../Core/Inc/obd2_decoder.h"
#include "../Core/Inc/can_frame_queue.h"

/* Helper: build a fake OBD-II Mode 01 response frame */
static can_capture_frame_t make_obd_frame(uint8_t pid, uint8_t A, uint8_t B)
{
    can_capture_frame_t f = {0};
    f.identifier  = 0x7E8u;
    f.extended    = 0u;
    f.remote      = 0u;
    f.dlc         = 8u;
    f.data[0]     = 0x04u; /* length */
    f.data[1]     = 0x41u; /* positive response */
    f.data[2]     = pid;
    f.data[3]     = A;
    f.data[4]     = B;
    return f;
}

/* Generated from SAE J1979 Mode 01 scaling, A=0x7B B=0x2D. */
static const struct { uint8_t pid; const char *line; } k_j1979[] = {
    { 0x03u, "PID,03,Fuel System Status,,7B2D\r\n" },
    { 0x04u, "PID,04,Engine Load,%,48.2\r\n" },
    { 0x05u, "PID,05,Coolant Temp,C,83\r\n" },
    { 0x06u, "PID,06,Fuel Trim ST B1,%,-3.91\r\n" },
    { 0x07u, "PID,07,Fuel Trim LT B1,%,-3.91\r\n" },
    { 0x08u, "PID,08,Fuel Trim ST B2,%,-3.91\r\n" },
    { 0x09u, "PID,09,Fuel Trim LT B2,%,-3.91\r\n" },
    { 0x0Au, "PID,0A,Fuel Pressure,kPa,369\r\n" },
    { 0x0Bu, "PID,0B,Intake Manifold Pressure,kPa,123\r\n" },
    { 0x0Cu, "PID,0C,RPM,rpm,7883\r\n" },
    { 0x0Du, "PID,0D,Vehicle Speed,kmh,123\r\n" },
    { 0x0Eu, "PID,0E,Timing Advance,deg,-2.5\r\n" },
    { 0x0Fu, "PID,0F,Intake Air Temp,C,83\r\n" },
    { 0x10u, "PID,10,MAF,gs,315.33\r\n" },
    { 0x11u, "PID,11,Throttle,%,48.2\r\n" },
    { 0x12u, "PID,12,Secondary Air,,7B\r\n" },
    { 0x13u, "PID,13,O2 Sensors Present,,7B\r\n" },
    { 0x14u, "PID,14,O2 B1S1,V,0.615\r\n" },
    { 0x15u, "PID,15,O2 B1S2,V,0.615\r\n" },
    { 0x16u, "PID,16,O2 B1S3,V,0.615\r\n" },
    { 0x17u, "PID,17,O2 B1S4,V,0.615\r\n" },
    { 0x18u, "PID,18,O2 B2S1,V,0.615\r\n" },
    { 0x19u, "PID,19,O2 B2S2,V,0.615\r\n" },
    { 0x1Au, "PID,1A,O2 B2S3,V,0.615\r\n" },
    { 0x1Bu, "PID,1B,O2 B2S4,V,0.615\r\n" },
    { 0x1Cu, "PID,1C,OBD Standard,,123\r\n" },
    { 0x1Fu, "PID,1F,Engine Runtime,s,31533\r\n" },
    { 0x21u, "PID,21,Distance MIL On,km,31533\r\n" },
    { 0x22u, "PID,22,Fuel Rail Pressure,kPa,2491.1\r\n" },
    { 0x23u, "PID,23,Fuel Rail Gauge Pressure,kPa,315330\r\n" },
    { 0x24u, "PID,24,WBO2 B1S1,lambda,0.9623\r\n" },
    { 0x25u, "PID,25,WBO2 B1S2,lambda,0.9623\r\n" },
    { 0x26u, "PID,26,WBO2 B1S3,lambda,0.9623\r\n" },
    { 0x27u, "PID,27,WBO2 B1S4,lambda,0.9623\r\n" },
    { 0x28u, "PID,28,WBO2 B2S1,lambda,0.9623\r\n" },
    { 0x29u, "PID,29,WBO2 B2S2,lambda,0.9623\r\n" },
    { 0x2Au, "PID,2A,WBO2 B2S3,lambda,0.9623\r\n" },
    { 0x2Bu, "PID,2B,WBO2 B2S4,lambda,0.9623\r\n" },
    { 0x2Cu, "PID,2C,EGR Commanded,%,48.2\r\n" },
    { 0x2Du, "PID,2D,EGR Error,%,-3.91\r\n" },
    { 0x2Eu, "PID,2E,Evap Purge,%,48.2\r\n" },
    { 0x2Fu, "PID,2F,Fuel Level,%,48.2\r\n" },
    { 0x30u, "PID,30,Warm-ups Since Clear,,123\r\n" },
    { 0x31u, "PID,31,Distance Since Clear,km,31533\r\n" },
    { 0x32u, "PID,32,Evap Vapour Pressure,Pa,7883.25\r\n" },
    { 0x33u, "PID,33,Barometric Pressure,kPa,123\r\n" },
    { 0x3Cu, "PID,3C,Catalyst Temp B1S1,C,3113.3\r\n" },
    { 0x3Du, "PID,3D,Catalyst Temp B2S1,C,3113.3\r\n" },
    { 0x3Eu, "PID,3E,Catalyst Temp B1S2,C,3113.3\r\n" },
    { 0x3Fu, "PID,3F,Catalyst Temp B2S2,C,3113.3\r\n" },
    { 0x42u, "PID,42,Battery Voltage,V,31.533\r\n" },
    { 0x43u, "PID,43,Abs Engine Load,%,12365.9\r\n" },
    { 0x44u, "PID,44,Commanded EQ Ratio,lambda,0.9623\r\n" },
    { 0x45u, "PID,45,Relative Throttle,%,48.2\r\n" },
    { 0x46u, "PID,46,Ambient Air Temp,C,83\r\n" },
    { 0x47u, "PID,47,Throttle Pos B,%,48.2\r\n" },
    { 0x49u, "PID,49,Accel Pedal D,%,48.2\r\n" },
    { 0x4Au, "PID,4A,Accel Pedal E,%,48.2\r\n" },
    { 0x4Cu, "PID,4C,Commanded Throttle,%,48.2\r\n" },
    { 0x4Du, "PID,4D,Time MIL On,min,31533\r\n" },
    { 0x4Eu, "PID,4E,Time Since Clear,min,31533\r\n" },
    { 0x5Cu, "PID,5C,Oil Temp,C,83\r\n" },
    { 0x5Eu, "PID,5E,Fuel Rate,Lh,1576.65\r\n" },
};

int main(void)
{
    /* Every decoded PID against the J1979 formula, plus filtering and
     * truncation guards. */
    for (size_t i = 0; i < sizeof k_j1979 / sizeof k_j1979[0]; i++) {
        char l[OBD2_LINE_CAPACITY];
        can_capture_frame_t f = make_obd_frame(k_j1979[i].pid, 0x7B, 0x2D);
        if (!obd2_decode(&f, l, sizeof l) || strcmp(l, k_j1979[i].line) != 0) {
            fprintf(stderr, "PID %02X: got [%s] want [%s]\n", k_j1979[i].pid, l, k_j1979[i].line);
            return 1;
        }
    }
    {
        char l[OBD2_LINE_CAPACITY];
        can_capture_frame_t f = make_obd_frame(0x0C, 0x7B, 0x2D);
        f.data[0] = 0x03u;                         /* 2-byte PID with 1 byte: short */
        assert(!obd2_decode(&f, l, sizeof l));
        f = make_obd_frame(0x0C, 0x7B, 0x2D); f.data[0] = 0x13u;   /* not SF */
        assert(!obd2_decode(&f, l, sizeof l));
        f = make_obd_frame(0x0C, 0x7B, 0x2D); f.data[1] = 0x7Fu;   /* not 0x41 */
        assert(!obd2_decode(&f, l, sizeof l));
        f = make_obd_frame(0x0C, 0x7B, 0x2D); f.dlc = 3u;
        assert(!obd2_decode(&f, l, sizeof l));
        f = make_obd_frame(0x0C, 0x7B, 0x2D); f.remote = 1u;
        assert(!obd2_decode(&f, l, sizeof l));
        assert(!obd2_decode(NULL, l, sizeof l));
    }

    char line[OBD2_LINE_CAPACITY];

    /* PID 0x0C — RPM */
    {
        can_capture_frame_t f = make_obd_frame(0x0C, 0x0E, 0x88); /* (0x0E*256+0x88)/4 = 3746/4 = 936.5 */
        assert(obd2_decode(&f, line, sizeof line));
        assert(strncmp(line, "PID,0C,RPM,rpm,", 15) == 0);
    }

    /* PID 0x05 — Coolant Temp: A=0x80=128 → 128-40=88°C */
    {
        can_capture_frame_t f = make_obd_frame(0x05, 0x80, 0x00);
        assert(obd2_decode(&f, line, sizeof line));
        assert(strncmp(line, "PID,05,Coolant Temp,C,", 22) == 0);
    }

    /* PID 0x42 — Battery Voltage: A=0x0C, B=0xCB → (12*256+203)/1000 = 3275/1000 = 3.275 V
     * (real battery voltage is higher but formula is (A*256+B)/1000) */
    {
        can_capture_frame_t f = make_obd_frame(0x42, 0x35, 0x4A); /* (0x35*256+0x4A)/1000 = (53*256+74)/1000 = 13642/1000 = 13.64 V */
        assert(obd2_decode(&f, line, sizeof line));
        assert(strncmp(line, "PID,42,Battery Voltage,V,", 25) == 0);
    }

    /* PID 0x2F — Fuel Level: A=0x47=71 → 71*100/255 ≈ 27.8% */
    {
        can_capture_frame_t f = make_obd_frame(0x2F, 0x47, 0x00);
        assert(obd2_decode(&f, line, sizeof line));
        assert(strncmp(line, "PID,2F,Fuel Level,%,", 20) == 0);
    }

    /* PID 0x11 — Throttle: A=0x40=64 → 64*100/255 ≈ 25.1% */
    {
        can_capture_frame_t f = make_obd_frame(0x11, 0x40, 0x00);
        assert(obd2_decode(&f, line, sizeof line));
        assert(strncmp(line, "PID,11,Throttle,%,", 18) == 0);
    }

    /* PID 0x10 — MAF: A=0x02, B=0x58 → (2*256+88)/100 = 5.96 g/s */
    {
        can_capture_frame_t f = make_obd_frame(0x10, 0x02, 0x58);
        assert(obd2_decode(&f, line, sizeof line));
        assert(strncmp(line, "PID,10,MAF,gs,", 14) == 0);
    }

    /* Unknown PID — should return false */
    {
        can_capture_frame_t f = make_obd_frame(0xAA, 0x00, 0x00);
        assert(!obd2_decode(&f, line, sizeof line));
    }

    /* Wrong ID — not 0x7E8 */
    {
        can_capture_frame_t f = make_obd_frame(0x0C, 0x10, 0x00);
        f.identifier = 0x7E0u;
        assert(!obd2_decode(&f, line, sizeof line));
    }

    /* Wrong mode byte — not 0x41 */
    {
        can_capture_frame_t f = make_obd_frame(0x0C, 0x10, 0x00);
        f.data[1] = 0x40u;
        assert(!obd2_decode(&f, line, sizeof line));
    }

    /* NULL inputs */
    assert(!obd2_decode(NULL, line, sizeof line));
    {
        can_capture_frame_t f = make_obd_frame(0x0C, 0x10, 0x00);
        assert(!obd2_decode(&f, NULL, sizeof line));
        assert(!obd2_decode(&f, line, 0));
        assert(!obd2_decode(&f, line, OBD2_LINE_CAPACITY - 1u)); /* too small */
    }

    /* Not a single frame / too short: never decoded as a value */
    {
        can_capture_frame_t f = make_obd_frame(0x0C, 0x0E, 0x88);
        f.data[0] = 0x10u;                       /* first frame PCI */
        assert(!obd2_decode(&f, line, sizeof line));
        f.data[0] = 0x21u;                       /* consecutive frame PCI */
        assert(!obd2_decode(&f, line, sizeof line));
        f.data[0] = 0x03u;                       /* SF: 41 0C A only, B is padding */
        assert(!obd2_decode(&f, line, sizeof line));
        f.data[0] = 0x08u;                       /* SF length larger than DLC-1 */
        assert(!obd2_decode(&f, line, sizeof line));
        f.data[0] = 0x04u;                       /* valid: 41 0C A B */
        assert(obd2_decode(&f, line, sizeof line));
        can_capture_frame_t g = make_obd_frame(0x05, 0x80, 0x00);
        g.data[0] = 0x03u;                       /* valid 1-byte PID */
        assert(obd2_decode(&g, line, sizeof line));
    }

    /* PIDs 0x66 / 0x67: layout not in a verified source, never decoded */
    {
        can_capture_frame_t f = make_obd_frame(0x66, 0x03, 0x01);
        f.data[0] = 0x07u; f.data[5] = 0x00u; f.data[6] = 0x02u; f.data[7] = 0x00u;
        assert(!obd2_decode(&f, line, sizeof line));
        can_capture_frame_t g = make_obd_frame(0x67, 0x03, 0x5A);
        g.data[0] = 0x05u; g.data[5] = 0x64u;
        assert(!obd2_decode(&g, line, sizeof line));
    }

    puts("PASS: obd2_decoder — all PID formulas, edge cases, and null guards");
    return 0;
}
