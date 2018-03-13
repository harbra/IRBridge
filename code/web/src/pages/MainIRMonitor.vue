<!--
This vue.js file is the main display for detected IR commands.
-->
<template>
<div id="app" class="is-marginless">
  <TopNav title="IR Monitor" subtitle="Monitor IR commands that are detected by your IRBridge."
  :sublinks="[{linkto:'/irmonitor', linktext:'IR Monitor'},
              {linkto:'/devices', linktext:'Devices'},
              {linkto:'/bookmarks', linktext:'Bookmarks'}]"></TopNav>
  <section class="section">
    <table class="table is-narrow is-fullwidth is-hoverable">
      <col width="33%">
      <col width="33%">
      <col width="33%">
      <col>
      <thead>
        <tr>
          <th><abbr>Type</abbr></th>
          <th><abbr>Command</abbr></th>
          <th><abbr>Options</abbr></th>
          <th></th>
        </tr>
      </thead>
      <tbody>
        <tr v-if="irevents.length==0">
          <td colspan="4">
            <div class="level">
              <div class="level-item has-text-centered">
                <div>
                  <p class="title">No IR event detected yet.</p>
                </div>
              </div>
            </div>
          </td>
        </tr>
        <tr v-for="irevent in invertedIREvents" v-else>
          <td>
            <template v-if="irevent.type>0 && irevent.type_String!='RAW'">
              <div class="tags has-addons">
                <span class="tag">Protocol</span>
                <span class="tag is-info">{{protocols[irevent.type]}}</span>
              </div>
            </template>
            <template v-else>
              <span class="tag is-primary">RAW</span>
            </template>
          </td>
          <td>
            <div class="field is-grouped is-grouped-multiline">
              <div class="control">
                <code>{{irevent.value.toString(16)}}</code>
              </div>
              <div class="control" v-if="irevent.address!=0">
                <div class="tags has-addons">
                  <span class="tag">Address</span>
                  <span class="tag is-info">{{irevent.address.toString(16)}}</span>
                </div>
              </div>
              <div class="control" v-if="irevent.command!=0">
                <div class="tags has-addons">
                  <span class="tag">Command</span>
                  <span class="tag is-info">{{irevent.command.toString(16)}}</span>
                </div>
              </div>
            </div>
          </td>
          <td>
            <div class="field is-grouped is-grouped-multiline">
              <div class="control">
                <div class="tags has-addons">
                  <span class="tag is-dark">{{irevent.bits}}</span>
                  <span class="tag">Bits</span>
                </div>
              </div>
              <div class="control" v-if="irevent.repeats>0">
                <div class="tags has-addons">
                  <span class="tag">Repeats</span>
                  <span class="tag is-info">{{irevent.repeats}}x</span>
                </div>
              </div>
              <span class="tag is-danger" v-if="irevent.overflow">Overflow</span>
            </div>
          </td>
          <td>
            <div class="buttons is-right">
              <a class="button is-small is-primary is-hidden-touch" v-on:click="startSaveAsBookmark(irevent)">
                <span class="icon">
                  <i class="icon-save"></i>
                </span>
                <span>Save</span>
              </a>
              <a class="button is-small is-primary is-hidden-desktop" v-on:click="startSaveAsBookmark(irevent)">
                <span class="icon">
                  <i class="icon-save"></i>
                </span>
              </a>
            </div>
          </td>
        </tr>
      </tbody>
    </table>
    <div class="modal is-active" v-if="saveAsModalShown">
      <div class="modal-background"></div>
      <div class="modal-content">
        <ModalName dialogTitle="Save as bookmark" v-on:cancel="saveAsModalShown=false" v-on:commit="saveAsBookmark"></ModalName>
      </div>
    </div>
  </section>
  <BottomFooter></BottomFooter>
</div>
</template>

<script>
import ModalName from '../components/ModalName.vue'
import WebEvents from '../plugins/webevents'

import {
  HTTP
} from '../plugins/axios'

export default {
  name: 'MainBookmarks',
  components: {
    ModalName
  },
  computed: {
    invertedIREvents: function() {
      return this.irevents.slice().reverse();
    }
  },
  data() {
    return {
      irevents: [],
      saveAsModalShown: false,
      activeIREvent: undefined,
      // the protocols are a copy from the protocols defined in IRRemoteESP8266, enum decode_type_t in IRRemoteESP8266.h
      protocols: ["UNUSED",
        "RC5",
        "RC6",
        "NEC",
        "SONY",
        "PANASONIC",
        "JVC",
        "SAMSUNG",
        "WHYNTER",
        "AIWA_RC_T501",
        "LG",
        "SANYO",
        "MITSUBISHI",
        "DISH",
        "SHARP",
        "COOLIX",
        "DAIKIN",
        "DENON",
        "KELVINATOR",
        "SHERWOOD",
        "MITSUBISHI_AC",
        "RCMM",
        "SANYO_LC7461",
        "RC5X",
        "GREE",
        "PRONTO", // Technically not a protocol, but an encoding.
        "NEC_LIKE",
        "ARGO",
        "TROTEC",
        "NIKAI",
        "RAW", // Technically not a protocol, but an encoding.
        "GLOBALCACHE", // Technically not a protocol, but an encoding.
        "TOSHIBA_AC",
        "FUJITSU_AC",
        "MIDEA",
        "MAGIQUEST",
        "LASERTAG",
        "CARRIER_AC"
      ]
    }
  },
  methods: {
    // shows the modal to create a name for a new bookmark
    startSaveAsBookmark: function(irevent) {
      this.activeIREvent=irevent;
      this.saveAsModalShown=true;
    },
    // callback function for the modal once the user commits
    saveAsBookmark: function(retval,loadingComponent) {
      var bookmark={};
      bookmark.name=retval.name;
      bookmark.repeats=this.activeIREvent.repeats;
      bookmark.type=((this.activeIREvent.type>0 && this.activeIREvent.type_String!='RAW') ? "P" : "R");
      bookmark.content={};
      if(bookmark.type=='P')
      {
        bookmark.content.prot=this.activeIREvent.type;
        bookmark.content.value=this.activeIREvent.value;
        bookmark.content.bits=this.activeIREvent.bits;
      } else if(bookmark.type=='R')
      {
        // raw not supported
      }

      HTTP.post('api/bookmarks',bookmark,{timeout: 5000}).then((response) => {
        this.$toast.open({
          message: 'Bookmark \"'+bookmark.name+'\" has been added.',
          type: 'is-success',
          position: 'is-bottom'
        });

        this.activeBookmark=undefined;
        this.saveAsModalShown=false;
      }).catch((e) => {
        if(e.response && e.response.status==409) {
          this.$toast.open({
            message: 'Bookmark \"'+bookmark.name+'\" already exists.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else if(e.response && e.response.status==400) {
          this.$toast.open({
            message: 'Request didn\'t contain all required information.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else {
          this.$toast.open({
            message: 'Unknown error creating bookmark \"'+bookmark.name+'\".',
            type: 'is-danger',
            position: 'is-bottom'
          });
        }
      }).then(() => {
        loadingComponent.close();
      })
    }
  },
  // on create, register a new server-sent events listener for new incoming IR codes
  created: function() {
    if(WebEvents.sourceAvailable()) {
      this.$eventsource.addEventListener('IRreceived', e => {
        try {
          var newevent=JSON.parse(e.data);
          if(!newevent.repeat) { newevent.repeats=0; this.irevents.push(newevent); }
          else { this.irevents[this.irevents.length-1].repeats++; }
        } catch (err) {
        }
      }, false);
    }
  }
}
</script>

<style scoped>

</style>
