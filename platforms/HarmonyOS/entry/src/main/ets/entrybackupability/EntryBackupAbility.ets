import { hilog } from '@kit.PerformanceAnalysisKit';
import { BackupExtensionAbility, BundleVersion } from '@kit.CoreFileKit';

export default class EntryBackupAbility extends BackupExtensionAbility {
  async onBackup() {
    hilog.info(0x0000, 'EngineXX', 'onBackup ok');
  }

  async onRestore(bundleVersion: BundleVersion) {
    hilog.info(0x0000, 'EngineXX', 'onRestore ok %{public}s', JSON.stringify(bundleVersion));
  }
}