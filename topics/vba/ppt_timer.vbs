Private Declare Sub Sleep Lib "kernel32.dll" (ByVal dwMilliseconds As Long)

Dim tmStartTime As Long
Dim tmFinishTime As Long
Dim tmMinute As Long
Dim tmSecond As Long

Sub TimerTest()

    Static isPlaying As Boolean
    If isPlaying Then
        Exit Sub
    End If
    isPlaying = True
    tmStartTime = Timer
    myTime = 0
    durationTime = 3
    If Application.SlideShowWindows.Count > 0 Then
        curSliderIdx = ActivePresentation.SlideShowWindow.View.Slide.SlideIndex
    End If
    Do
        DoEvents
        tmFinishTime = Timer
        If tmFinishTime - tmStartTime > myTime Then
            tmMinute = (durationTime - myTime) \ 60
            tmSecond = (durationTime - myTime) Mod 60
            Label1.Caption = Format(tmMinute, "0#") & ":" & Format(tmSecond, "0#")
            myTime = myTime + 1
            'If tmMinute < 1 And tmSecond < 31 Then
            '    Label1.Caption = Format(tmMinute, "0#") & ":" & Format(tmSecond, "0#")
            'Else
            '    Label1.Caption = Format(tmMinute + 1, "0#") & ":00"
            'End If
        End If
        Sleep (50)
        DoEvents
        If Application.SlideShowWindows.Count > 0 Then
            If ActivePresentation.SlideShowWindow.View.Slide.SlideIndex <> curSliderIdx Then
                Exit Do
            End If
        End If
        
    Loop Until (myTime = durationTime + 1) Or (Application.SlideShowWindows.Count = 0)
    
    isPlaying = False
    tmMinute = (durationTime) \ 60
    tmSecond = (durationTime) Mod 60
    Label1.Caption = Format(tmMinute, "0#") & ":" & Format(tmSecond, "0#")
    
End Sub

Private Sub Label1_Click()

    TimerTest

End Sub


