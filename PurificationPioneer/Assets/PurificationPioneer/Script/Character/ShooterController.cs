﻿using System.Collections.Generic;
using PurificationPioneer.Global;
using PurificationPioneer.Network.ProtoGen;
using PurificationPioneer.Scriptable;
using UnityEngine;

namespace PurificationPioneer.Script
{
    [RequireComponent(typeof(CharacterController))]
    public class ShooterController : PpCharacterController
    {
        #region SerializeFields

        [SerializeField] private ShootingSystem m_ShootSystem;

        #endregion
        


        #region Events

        protected override void Start()
        {
            base.Start();
            if (m_WorkAsLocal)
            {
                m_ShootSystem.Initialize(
                    1,
                    1,
                    () => GlobalVar.IsPlayerInControl && Input.GetMouseButton(0),
                    m_LocalCameraHelper.vcam,
                    GameSettings.Instance.LeftMaterial);
            }
        }

        #endregion
        

        #region IFrameSyncCharacter

        public override void SyncLastCharacterInput(IEnumerable<PlayerInput> inputs)
        {
            m_ShootSystem.ApplyAndSimulate();
            m_ShootSystem.SaveState();
            base.SyncLastCharacterInput(inputs);
        }
        
        #endregion


        protected override void InitCharacter(bool isLocal)
        {
            base.InitCharacter(isLocal);
            if (isLocal)
            {
                m_ShootSystem.Initialize(SeatId, PaintEfficiencyScale, () => m_Attacking, m_LocalCameraHelper.vcam);
            }
            else
            {
                m_ShootSystem.Initialize(SeatId, PaintEfficiencyScale, () => m_Attacking);
            }
        }
    }
}