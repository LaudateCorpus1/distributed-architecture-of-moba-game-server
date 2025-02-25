﻿using System.Collections.Generic;
using System.Linq;
using System.Text;
using PurificationPioneer.Const;
using PurificationPioneer.Global;
using PurificationPioneer.Network.Const;
using PurificationPioneer.Network.ProtoGen;
using PurificationPioneer.Script;
using PurificationPioneer.Scriptable;
using ReadyGamerOne.Common;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace PurificationPioneer.Network.Proxy
{
    public class LogicProxy:Singleton<LogicProxy>
    {
        private bool logicServerConnected = false;
        public LogicProxy()
        {
            NetworkMgr.Instance.AddNetMsgListener(ServiceType.Logic,OnLogicCmd);
        }

        private void OnLogicCmd(CmdPackageProtocol.CmdPackage package)
        {
            switch (package.cmdType)
            {
                #region LogicCmd.OnGameEndTick

                case LogicCmd.OnGameEndTick:
                    var gameEndTick = CmdPackageProtocol.ProtobufDeserialize<OnGameEndTick>(package.body);
                    if (null == gameEndTick)
                    {
                        Debug.LogError($"OnGameEndTick is null");
                        return;  
                    }

                    if (gameEndTick.status != Response.Ok)
                    {
                        Debug.LogError($"[OnGameEndTick] status: {gameEndTick.status}");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[OnGameEndTick] 比赛结束");
#endif
                    GlobalVar.SetGameEndInfo(gameEndTick);
                    SceneMgr.LoadSceneWithSimpleLoadingPanel(SceneName.GameEnd);
                    break;

                #endregion
                
                #region LogicCmd.ExitGameRes
                case LogicCmd.ExitGameRes:        
                    var exitGameRes = CmdPackageProtocol.ProtobufDeserialize<ExitGameRes>(package.body);
                    if (null == exitGameRes)
                    {
                        Debug.LogError($"ExitGameRes is null");
                        return;
                    }

                    if (exitGameRes.status != Response.Ok)
                    {
                        Debug.LogError($"[ExitGameRes] status: {exitGameRes.status}");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[ExitGameRes] 退出比赛");
#endif

                    Debug.Log($"[ExitGameRes] 退出比赛");
                    SceneMgr.LoadSceneWithSimpleLoadingPanel(SceneName.Home);
                    
                    break;
                
                #endregion

                #region LogicCmd.OnCharacterExitTick

                    
                case LogicCmd.OnCharacterExitTick:   
                    var onCharacterExit = CmdPackageProtocol.ProtobufDeserialize<OnCharacterExitTick>(package.body);
                    if (null == onCharacterExit)
                    {
                        Debug.LogError($"OnCharacterExitTick is null");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[OnCharacterExitTick] 玩家{GlobalVar.SeatId_MatcherInfo[onCharacterExit.seatId].Unick}退出比赛");
#endif
                    Debug.Log($" 玩家{GlobalVar.SeatId_MatcherInfo[onCharacterExit.seatId].Unick}退出比赛");
                    break;

                #endregion

                #region LogicCmd.StartStoryRes
                
                case LogicCmd.StartStoryRes:
                    Debug.Log($"[StartStoryRes] 故事模式开始");
                    break;

                #endregion
                
                #region LogicCmd.LogicFramesToSync
                case LogicCmd.LogicFramesToSync:
                    var logicFrameToSync = CmdPackageProtocol.ProtobufDeserialize<LogicFramesToSync>(package.body);
                    if (null == logicFrameToSync)
                    {
                        Debug.LogError($"LogicFramesToSync is null");
                        return;
                    }         
                    
#if DebugMode
                    if (GameSettings.Instance.EnableFrameSyncLog)
                    {
                        var inputCountList = new StringBuilder();
                        foreach (var frame in logicFrameToSync.unsyncFrames)
                        {
                            inputCountList.Append($" [{frame.frameId}-{frame.inputs.Count}]");
                        }
                        Debug.Log($"[LogicFramesToSync] 帧同步进行中[NewestInputId-{FrameSyncMgr.NewestInputId}][FrameId-{FrameSyncMgr.FrameId}][PackageFrameId-{logicFrameToSync.frameId}][Inputs-《{inputCountList}》]");                        
                    }
#endif
                    FrameSyncMgr.OnFrameSyncTick(logicFrameToSync, GlobalVar.LogicFrameDeltaTime);
                    break;
                #endregion
                
                #region LogicCmd.StartGameRes
                case LogicCmd.StartGameRes:
                    var startGameReq = CmdPackageProtocol.ProtobufDeserialize<StartGameRes>(package.body);
                    if (null == startGameReq)
                    {
                        Debug.LogError($"StartGameRes is null");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[StartGameRes] 游戏 {startGameReq.startGameDelay} 秒后开始");
#endif
                    GlobalVar.SaveGameInfos(startGameReq);
                    CEventCenter.BroadMessage(Message.OnGameStart, GlobalVar.StartGameDelay);
                    break;
                #endregion
                
                #region LogicCmd.StartLoadGame
                case LogicCmd.StartLoadGame:
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[StartLoadGame] 开始加载游戏");
#endif
                    CEventCenter.BroadMessage(Message.OnStartLoadGame);
                    break;
                #endregion

                #region LogicCmd.ForceSelect
                case LogicCmd.ForceSelect:
                    var forceSelect = CmdPackageProtocol.ProtobufDeserialize<ForceSelect>(package.body);
                    if (null == forceSelect)
                    {
                        Debug.LogError($"ForceSelect is null");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[ForceSelect] 强制选择默认英雄");
#endif
                    break;

                #endregion
                
                #region LogicCmd.UpdateSelectTimer
                case LogicCmd.UpdateSelectTimer:
                    var updateSelectTimer = CmdPackageProtocol.ProtobufDeserialize<UpdateSelectTimer>(package.body);
                    if (null == updateSelectTimer)
                    {
                        Debug.LogError($"UpdateSelectTimer is null");
                        return;
                    }

                    CEventCenter.BroadMessage(Message.OnUpdateSelectTimer, updateSelectTimer);
                    break;
                #endregion

                #region LogicCmd.SelectHeroRes
                case LogicCmd.SelectHeroRes:
                    var selectHeroRes = CmdPackageProtocol.ProtobufDeserialize<SelectHeroRes>(package.body);
                    if (null == selectHeroRes)
                    {
                        Debug.LogError($"SelectHeroRes is null");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[SelectHeroRes] SeatId[{selectHeroRes.seatId}] 选择了 HeroId[{selectHeroRes.hero_id}]");
#endif
                    GlobalVar.OnSelectHero(selectHeroRes);
                    CEventCenter.BroadMessage(Message.OnSelectHero, selectHeroRes);
                    
                    break;
                #endregion

                #region LogicCmd.SubmitHeroRes

                case LogicCmd.SubmitHeroRes:
                    var submitHeroRes = CmdPackageProtocol.ProtobufDeserialize<SubmitHeroRes>(package.body);
                    if(null==submitHeroRes)
                    {
                        Debug.LogError($"SubmitHeroRes is null");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[SubmitHeroRes] SeatId[{submitHeroRes.seatId}] 锁定英雄]");
#endif
                    GlobalVar.OnSubmitHero(submitHeroRes);
                    CEventCenter.BroadMessage(Message.OnSubmitHero, submitHeroRes);
                    
                    break;

                #endregion
                
                #region LogicCmd.FinishMatchTick
                case LogicCmd.FinishMatchTick:
                    var finishMatchTick = CmdPackageProtocol.ProtobufDeserialize<FinishMatchTick>(package.body);
                    if (null == finishMatchTick)
                    {
                        Debug.LogError($"FinishMatchTick is null");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[FinishMatchTick]匹配完成({finishMatchTick.matchers.Count})，开始英雄选择");
#endif
                    GlobalVar.SaveMatchers(finishMatchTick);
                    CEventCenter.BroadMessage(Message.OnFinishMatch);
                    
                    break;
                #endregion
                
                #region LogicCmd.StopMatchRes

                case LogicCmd.StopMatchRes:
                    var stopMatchRes = CmdPackageProtocol.ProtobufDeserialize<StopMatchRes>(package.body);
                    if (stopMatchRes == null)
                    {
                        Debug.LogError($"stopMatchRes is null");
                        return;
                    }
                    if (stopMatchRes.status == Response.Ok)
                    {
#if DebugMode
                        if(GameSettings.Instance.EnableProtoLog)
                            Debug.Log($"[StopMatchRes]取消匹配");
#endif
                        CEventCenter.BroadMessage(Message.OnStopMatch);
                    }
                    
                    break;                

                #endregion

                #region LogicCmd.RemoveMatcherTick

                case LogicCmd.RemoveMatcherTick:
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[RemoveMatcherTick]有玩家走了");
#endif
                    CEventCenter.BroadMessage(Message.OnRemovePlayer);
                    break;                 

                #endregion

                #region LogicCmd.AddMatcherTick

                case LogicCmd.AddMatcherTick:
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[AddMatcherTick]新玩家来了");
#endif
                    CEventCenter.BroadMessage(Message.OnAddPlayer);
                    break;                      

                #endregion

                #region LogicCmd.StartMatchRes

                case LogicCmd.StartMatchRes:
                    var startMatchRes = CmdPackageProtocol.ProtobufDeserialize<StartMatchRes>(package.body);
                    if (startMatchRes == null)
                    {
                        Debug.LogError($"startMatchRes is null");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[StartMatchRes]开始匹配:{startMatchRes.current}/{startMatchRes.max}");
#endif
                    CEventCenter.BroadMessage(Message.OnStartMatch,startMatchRes);
                    break;                    

                #endregion

                #region LogicCmd.InitUdpRes

                case LogicCmd.InitUdpRes:
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[InitUdpRes] Udp初始化完成");
#endif
                    break;   

                #endregion
                
                #region LogicCmd.LoginLogicRes

                case LogicCmd.LoginLogicRes:
                    var loginLogicRes = CmdPackageProtocol.ProtobufDeserialize<LoginLogicRes>(package.body);
                    if (loginLogicRes == null)
                    {
                        Debug.LogError($"loginLogicRes is null");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[LoginLogicRes]登陆逻辑服务器成功");
#endif
                    //开启Udp接收
                    GameSettings.Instance.SetUdpServerIp(loginLogicRes.udp_ip);
                    GameSettings.Instance.SetUdpServerPort(loginLogicRes.udp_port);
                    NetworkMgr.Instance.SetupUdp(
                        (status)=>
                        {
                            logicServerConnected = status;
                            if (logicServerConnected)
                            {
                                Debug.Log("Udp服务建立成功");
                                NetworkMgr.Instance.UdpSendProtobuf(ServiceType.Logic, LogicCmd.InitUdpReq, new InitUdpReq{uname = GlobalVar.Uname});
                            }
                            else 
                                Debug.LogWarning("Udp服务建立失败");
                        });
                    break;                    

                #endregion

                #region LogicCmd.UdpTestRes

                case LogicCmd.UdpTestRes:
                    var udpTestRes = CmdPackageProtocol.ProtobufDeserialize<UdpTestRes>(package.body);
                    if (udpTestRes == null)
                    {
                        Debug.LogError($"UdpTestRes is null");
                        return;
                    }
#if DebugMode
                    if(GameSettings.Instance.EnableProtoLog)
                        Debug.Log($"[UdpTestRes]{udpTestRes.content}");
#endif
                    break;                    

                #endregion
            }
        }
        
        /// <summary>
        /// 登陆逻辑服务器
        /// </summary>
        public void Login()
        {
            NetworkMgr.Instance.TcpSendProtobuf(
                ServiceType.Logic,
                LogicCmd.LoginLogicReq,
                new LoginLogicReq{uname = GlobalVar.Uname});     
        }

        /// <summary>
        /// 测试Udp
        /// </summary>
        /// <param name="content"></param>
        public void TestUdp(string content)
        {
            if(!logicServerConnected)
                Debug.LogError($"尚未登陆逻辑服务器");
            
            var udpTestReq = new UdpTestReq
            {
                content = content,
            };
            NetworkMgr.Instance.UdpSendProtobuf(
                ServiceType.Logic,
                LogicCmd.UdpTestReq,
                udpTestReq);
        }

        /// <summary>
        /// 开始单人模式
        /// </summary>
        /// <param name="uname"></param>
        public void StartMatchSingle(string uname)
        {
            if (!logicServerConnected)
            {
                Debug.LogError($"尚未登陆逻辑服务器");
                return;
            }

            var roomType = LogicCmd.StartMatchReq;
            var startMatchReq = new StartMatchReq
            {
                uname = uname
            };

            GlobalVar.SetRoomType(roomType);
            NetworkMgr.Instance.TcpSendProtobuf(
                ServiceType.Logic,
                roomType,
                startMatchReq);
        }

        /// <summary>
        /// 开始故事模式
        /// </summary>
        /// <param name="uname"></param>
        public void StartStoryMode(string uname)
        {
            var roomType = LogicCmd.StartStoryReq;
            var startMatchReq = new StartStoryReq
            {
                uname = uname
            };

            GlobalVar.SetRoomType(roomType);
            NetworkMgr.Instance.TcpSendProtobuf(
                ServiceType.Logic,
                roomType,
                startMatchReq);
        }

        /// <summary>
        /// 开始多人模式
        /// </summary>
        /// <param name="uname"></param>
        public void StartMatchMulti(string uname)
        {
            if (!logicServerConnected)
            {
                Debug.LogError($"尚未登陆逻辑服务器");
                return;
            }

            var roomType = LogicCmd.StartMultiReq;
            var startMatchReq = new StartMultiReq
            {
                uname = uname
            };

            GlobalVar.SetRoomType(roomType);
            NetworkMgr.Instance.TcpSendProtobuf(
                ServiceType.Logic,
                roomType,
                startMatchReq);
        }

        /// <summary>
        /// 取消匹配
        /// </summary>
        /// <param name="uname"></param>
        public void TryStopMatch(string uname)
        {
            if(!logicServerConnected)
                Debug.LogError($"尚未登陆逻辑服务器");
            var stopMatchReq = new StopMatchReq
            {
                uname = uname,
            };
            NetworkMgr.Instance.TcpSendProtobuf(
                ServiceType.Logic,
                LogicCmd.StopMatchReq,
                stopMatchReq);
        }


        /// <summary>
        /// 选中英雄
        /// </summary>
        /// <param name="uname"></param>
        /// <param name="heroId"></param>
        public void SelectHero(string uname, int heroId)
        {
            var selectHeroReq = new SelectHeroReq
            {
                uname = uname,
                hero_id = heroId,
            };
            NetworkMgr.Instance.TcpSendProtobuf(
                ServiceType.Logic,
                LogicCmd.SelectHeroReq,
                selectHeroReq);
        }

        /// <summary>
        /// 锁定英雄
        /// </summary>
        /// <param name="uname"></param>
        public void SubmitHero(string uname)
        {
            NetworkMgr.Instance.TcpSendProtobuf(
                ServiceType.Logic,
                LogicCmd.SubmitHeroReq,
                new SubmitHeroReq{uname = uname});
        }

        /// <summary>
        /// 开始战斗
        /// </summary>
        /// <param name="uname"></param>
        public void StartGameReq(string uname)
        {
            var startGameReq = new StartGameReq
            {
                uname = uname,
            };
            NetworkMgr.Instance.TcpSendProtobuf(ServiceType.Logic, LogicCmd.StartGameReq, startGameReq);
        }

        /// <summary>
        /// 发送逻辑帧输入
        /// </summary>
        /// <param name="frameId"></param>
        /// <param name="roomType"></param>
        /// <param name="roomId"></param>
        /// <param name="seatId"></param>
        /// <param name="inputs"></param>
        public void SendLogicInput(int frameId,int roomType,int roomId,int seatId,
            IEnumerable<PlayerInput> inputs)
        {
            var nextFrameInput = new NextFrameInput
            {
                frameId = frameId,
                roomId = roomId,
                roomType = roomType,
                seatId = seatId,
            };
            foreach (var playerInput in inputs)
            {
                nextFrameInput.inputs.Add(playerInput);
            }
#if DebugMode
            if (GameSettings.Instance.EnableInputLog)
            {
                Debug.Log($"[InputLog] SendInput [inputFrameId-{frameId}][InputCount-{inputs.Count()}]");
            }
#endif
            NetworkMgr.Instance.UdpSendProtobuf(
                ServiceType.Logic,
                LogicCmd.NextFrameInput,
                nextFrameInput);
        }


        /// <summary>
        /// 尝试退出游戏
        /// </summary>
        /// <param name="uname"></param>
        public void TryExitGame(string uname)
        {
            var exitGameReq = new ExitGameReq
            {
                uname = uname
            };
            NetworkMgr.Instance.TcpSendProtobuf(ServiceType.Logic, LogicCmd.ExitGameReq, exitGameReq);
        }
    }
}