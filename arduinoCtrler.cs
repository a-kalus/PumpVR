using UnityEngine;
using System.IO.Ports;
public class arduinoCtrler : MonoBehaviour
{
    public string portName;
    public bool disableWeightSim;
    SerialPort sp;
    float next_time;

    public enum FillLevel
    {
        Empty=0,
        VeryLow=1,
        Low=2,
        Medium=3,
        High=4,
        Full=5
    }

    private FillLevel FillLevelLeft = FillLevel.Empty;
    private FillLevel FillLevelRight = FillLevel.Empty;

    void Start()
    {   if (disableWeightSim) { return; }
        string the_com = "";
        next_time = Time.time;

        foreach (string mysps in SerialPort.GetPortNames())
        {
            print(mysps);
            if (mysps == portName) { the_com = mysps; break; }
        }
        sp = new SerialPort("\\\\.\\" + the_com, 9600);
        if (!sp.IsOpen)
        {
            print("Opening " + the_com + ", baud 9600");
            sp.Open();
            sp.ReadTimeout = 100;
            sp.Handshake = Handshake.None;
            if (sp.IsOpen) { print("Open"); }
        }
    }
    

    void SendEvent(string s)
    {
        if (disableWeightSim) { return; }
        if (!sp.IsOpen)
        {
            sp.Open();
            print("opened sp");
        }
        if (sp.IsOpen)
        {
            print("Writing "+s);
            sp.Write((s));
        }
    }

    public void FillLeft(FillLevel lvl) {
        if (FillLevel.Empty < lvl && lvl <= FillLevel.Full)
        {
            string call = "L".ToString() + (int) lvl;
            SendEvent(call);
            FillLevelLeft = lvl; print("fill left"); print(lvl);
        }
    }
    public void FillRight(FillLevel lvl) {
        if (FillLevel.Empty < lvl && lvl <= FillLevel.Full)
        {
            string call = "R".ToString() + (int)lvl;
            SendEvent(call);
            FillLevelRight = lvl; print("fill right"); print(lvl);
        }
    }
    public void FillBoth(FillLevel lvl) {
        if (FillLevel.Empty < lvl && lvl <= FillLevel.Full)
        {
            string call = "B".ToString() + (int)lvl;
            SendEvent(call);
            FillLevelLeft = lvl; FillLevelRight = lvl; print("fill both"); print(lvl);
        }
    }
    public void DrainLeft(FillLevel lvl) {
        if (FillLevel.Empty < lvl && lvl <= FillLevel.Full)
        {
            string call = "l".ToString() + (int)lvl;
            SendEvent(call);
            FillLevelLeft = FillLevel.Empty; print("drain left");
        }
    }
    public void DrainRight(FillLevel lvl)
    {
        if (FillLevel.Empty < lvl && lvl <= FillLevel.Full)
        {
            string call = "r".ToString() + (int)lvl;
            SendEvent(call);
            FillLevelRight = FillLevel.Empty; print("drain right");
        }
    }
    public void DrainBoth(FillLevel lvl)
    {
        if (FillLevel.Empty < lvl && lvl <= FillLevel.Full)
        {
            string call = "b".ToString() + (int)lvl;
            SendEvent(call);
            FillLevelLeft = FillLevel.Empty; FillLevelRight = FillLevel.Empty;
        }
    }

    public FillLevel GetLeftFillLevel() { return FillLevelLeft; }
    public FillLevel GetRightFillLevel() { return FillLevelRight; }

}
