local CmdType=require("logic/const/CmdType");

roomInfos={}
roomInfos[CmdType.StartMatchReq]={
    Max=1,
    GameTime=500,
}


return {
    udp_ip="121.196.178.141",
    udp_port=10000,
    room_infos=roomInfos,
    heroSelectTime=20,
    finishSelectDelay=2,

    startGameDelay=3,

    logicFrameDeltaTime=50,--ms

    broadTimes=1,
};