<template>
<div id="app" class="is-marginless">
  <TopNav title="Settings - Time" subtitle="Time settings of the IRBridge can be configured here"
  :sublinks="[{linkto:'/settings/network', linktext:'Network'},
              {linkto:'/settings/backup', linktext:'Backup/Restore'}]"></TopNav>
  <section class="section">
    <div class="container">
      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Timezone</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <span class="select">
      <select v-model="SettingsTime.Timezone">
        <option>GMT+01</option>
        <option>GMT</option>
        <option>EST</option>
      </select>
    </span>
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal is-narrow">
        <div class="field-label">
          <!-- Left empty for spacing -->
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input type="checkbox" v-model="SettingsTime.useDST">
                Use DST?
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">NTP server</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input class="input" type="url" placeholder="NTP server URL" v-model="SettingsTime.NTPserver">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label">
          <!-- Left empty for spacing -->
        </div>
        <div class="field-body">
          <div class="field">
            <div class="control">
              <button class="button is-primary">
                <span class="icon">
                  <i class="icon-save"></i>
                </span>
                <span>Save</span>
              </button>
            </div>
          </div>
        </div>
      </div>

    </div>
  </section>
  <BottomFooter></BottomFooter>
</div>
</template>

<script>
import {HTTP} from '../plugins/axios'

export default {
  name: 'SettingsTime',
  components: {
  },
  data() {
    return {
      SettingsTime: {}
    }
  },
    created: function() {

    }
}
</script>

<style scoped>
</style>
