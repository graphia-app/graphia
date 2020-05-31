var INSTALL_COMPONENTS =
[
    "qt.qt5.5142.win64_msvc2017_64",
    "qt.qt5.5142.qtwebengine"
];

function clickNext()
{
    console.log(gui.currentPageWidget());
    gui.clickButton(buttons.NextButton, 1000);
}

function Controller()
{
    installer.setMessageBoxAutomaticAnswer("OverwriteTargetDirectory", QMessageBox.Yes);
    installer.installationFinished.connect(function()
    {
        clickNext();
    });
}

Controller.prototype.WelcomePageCallback = clickNext;
Controller.prototype.IntroductionPageCallback = clickNext;
Controller.prototype.CredentialsPageCallback = clickNext;
Controller.prototype.TargetDirectoryPageCallback = clickNext;

Controller.prototype.ComponentSelectionPageCallback = function()
{
    var page = gui.currentPageWidget();
    page.deselectAll();

    for(var i = 0; i < INSTALL_COMPONENTS.length; i++)
        page.selectComponent(INSTALL_COMPONENTS[i]);

    clickNext();
};

Controller.prototype.LicenseAgreementPageCallback = function()
{
    gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
    clickNext();
};

Controller.prototype.StartMenuDirectoryPageCallback = clickNext;
Controller.prototype.ReadyForInstallationPageCallback = clickNext;

Controller.prototype.FinishedPageCallback = function()
{
    var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm;

    if(checkBoxForm && checkBoxForm.launchQtCreatorCheckBox)
        checkBoxForm.launchQtCreatorCheckBox.checked = false;

    gui.clickButton(buttons.FinishButton);
};
