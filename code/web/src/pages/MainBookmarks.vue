<!--
This vue.js file is the main display for existing bookmarks.
-->
<template>
<div id="app" class="is-marginless">
  <TopNav title="Bookmarks" subtitle="Manage named bookmarks for IR commands."
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
          <th><abbr>Name</abbr></th>
          <th><abbr>Type</abbr></th>
          <th><abbr>Repeats</abbr></th>
          <th></th>
        </tr>
      </thead>
      <tbody>
        <tr v-if="sortedBookmarks.length==0">
          <td colspan="4">
            <div class="level">
              <div class="level-item has-text-centered">
                <div>
                  <p class="title">No bookmark found</p>
                </div>
              </div>
            </div>
          </td>
        </tr>
        <tr style="cursor: pointer;" v-for="bookmark in sortedBookmarks" v-else>
          <td>
            {{bookmark.name}}
          </td>
          <td>
            <template v-if="bookmark.type==='P'">
              <div class="tags has-addons">
                <span class="tag">Protocol</span>
                <span class="tag is-info">{{protocols[bookmark.content.prot]}}</span>
              </div>
            </template>
            <template v-else>
              <span class="tag is-primary">RAW</span>
            </template>
          </td>
          <td>
            <span class="tag" v-if="bookmark.repeats>0">{{bookmark.repeats}}x</span>
            <span class="tag is-light" v-else>none</span>
          </td>
          <td>
            <div class="field has-addons is-right">
              <a class="control button is-small is-primary is-hidden-touch" v-on:click="sendBookmark(bookmark)">
                <span class="icon">
                  <i class="icon-send"></i>
                </span>
                <span>Send</span>
              </a>
              <a class="control button is-small is-primary is-hidden-desktop" v-on:click="sendBookmark(bookmark)">
                <span class="icon">
                  <i class="icon-send"></i>
                </span>
              </a>


              <a class="control button is-small is-hidden-touch" v-on:click="startEditBookmark(bookmark)">
                <span class="icon">
                  <i class="icon-gear"></i>
                </span>
                <span>Edit</span>
              </a>
              <a class="control button is-small is-hidden-desktop" v-on:click="startEditBookmark(bookmark)">
                <span class="icon">
                  <i class="icon-gear"></i>
                </span>
              </a>

              <a class="control button is-small is-danger is-hidden-touch" v-on:click="deleteBookmark(bookmark)">
                <span class="icon">
                  <i class="icon-minus"></i>
                </span>
                <span>Delete</span>
              </a>
              <a class="control button is-small is-danger is-hidden-desktop" v-on:click="deleteBookmark(bookmark)">
                <span class="icon">
                  <i class="icon-minus"></i>
                </span>
              </a>

            </div>
          </td>
        </tr>
      </tbody>
    </table>
    <div class="level">
      <div class="level-item">
        <a class="button is-primary" v-on:click="startAddBookmark()">
          <span class="icon">
            <i class="icon-plus"></i>
          </span>
          <span>Add new</span>
        </a>
      </div>
    </div>
    <div class="modal is-active" v-if="editModalShown">
      <div class="modal-background"></div>
      <div class="modal-content">
        <ModalBookmark :protocols="protocols" :name="activeBookmark.name" :type="activeBookmark.type" :repeats="activeBookmark.repeats" :content="activeBookmark.content" dialogTitle="Edit Bookmark" v-on:cancel="editModalShown=false" v-on:commit="editBookmark"></ModalBookmark>
      </div>
    </div>
    <div class="modal is-active" v-else-if="addModalShown">
      <div class="modal-background"></div>
      <div class="modal-content">
        <ModalBookmark :protocols="protocols" dialogTitle="Add Bookmark" v-on:cancel="addModalShown=false" v-on:commit="addBookmark"></ModalBookmark>
      </div>
    </div>
  </section>
  <BottomFooter></BottomFooter>
</div>
</template>

<script>
import ModalBookmark from '../components/ModalBookmark.vue'

import {
  HTTP
} from '../plugins/axios'

export default {
  name: 'MainBookmarks',
  components: {
    ModalBookmark
  },
  computed: {
    // resort bookmarks based on the property .name
    sortedBookmarks: function() {
      return this.bookmarks.sort(function (a, b) {
        if (a.name < b.name) {
          return -1;
        }
        if (a.name > b.name) {
          return 1;
        }

        // otherwise a.name equals b.name
        return 0;
      });
    }
  },
  data() {
    return {
      editModalShown: false,
      addModalShown: false,
      activeBookmark: undefined,
      bookmarks: [],
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
    // called to delete a bookmark.
    // "bookmark" needs to be in the same format as returned by the ESP8266
    deleteBookmark: function(bookmark) {
      if (!bookmark.hasOwnProperty("name")) {
        return false;
      }

      var loadingComponent=this.$loading.open();

      // call the HTTP_DELETE method on /api/bookmarks/[bookmarkname]
      HTTP.delete('/api/bookmarks/'+bookmark.name,{timeout:5000}).then((response) => {
        this.$toast.open({
          message: 'Bookmark \"'+bookmark.name+'\" has been deleted.',
          type: 'is-success',
          position: 'is-bottom'
        });

        // if deletion was successful, don't re-get the bookmarks from the ESP, but instead just
        // delete the object from the bookmarks array to save processing power on the ESP.
        var index = this.bookmarks.indexOf(bookmark);
        if (index > -1) {
          this.bookmarks.splice(index, 1);
        }
      }).catch((e) => {
        // in case of an error, display an error message
        if(e.response && e.response.status==404) {
          this.$toast.open({
            message: 'Bookmark \"'+bookmark.name+'\" has not been found.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else if(e.response && e.response.status==423) {
          this.$toast.open({
            message: 'Bookmark \"'+bookmark.name+'\" is used in device \"'+e.response.data+'\". Delete aborted.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else {
          this.$toast.open({
            message: 'Unknown error deleting bookmark \"'+bookmark.name+'\".',
            type: 'is-danger',
            position: 'is-bottom'
          });
        }
      }).then(() => {
        loadingComponent.close();
      })
    },
    // calls the appropriate api function to send the IR command for the bookmark
    sendBookmark: function(bookmark) {
      HTTP.get('api/sendir?bookmark='+bookmark.name,{timeout: 5000}).then((response) => {
        this.$toast.open({
          message: 'IR code for \"'+bookmark.name+'\" has been sent.',
          type: 'is-success',
          position: 'is-bottom'
        });
      }).catch((e) => {
        if(e.response && e.response.status==404) {
          this.$toast.open({
            message: 'Bookmark \"'+bookmark.name+'\" not found.',
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
            message: 'Unknown error sending bookmark \"'+bookmark.name+'\".',
            type: 'is-danger',
            position: 'is-bottom'
          });
        }
      })
    },
    // used to display the edit modal to start editing a bookmark
    startEditBookmark: function(bookmark) {
      this.activeBookmark=bookmark;
      this.editModalShown=true;
    },
    // this is the callback function used by the edit modal to actually send the api command to edit the bookmark
    editBookmark: function(bookmark,loadingComponent) {
      HTTP.put('api/bookmarks/'+this.activeBookmark.name,bookmark,{timeout: 5000}).then((response) => {
        this.$toast.open({
          message: 'Bookmark \"'+this.activeBookmark.name+'\" has been modified.',
          type: 'is-success',
          position: 'is-bottom'
        });

        // if editing the bookmark succeeded, don't send another HTTP request to the ESP, but update
        // the bookmarks array automatically
        var index = this.bookmarks.indexOf(this.activeBookmark);
        if (index > -1) {
          this.bookmarks.splice(index, 1);
        }
        this.bookmarks.push(bookmark);

        this.activeBookmark=undefined;
        this.editModalShown=false;
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
            message: 'Unknown error editing bookmark \"'+this.activeBookmark.name+'\".',
            type: 'is-danger',
            position: 'is-bottom'
          });
        }
      }).then(() => {
        loadingComponent.close();
      })
    },
    // used to display the modal to add a new bookmark
    startAddBookmark: function() {
      this.activeBookmark=undefined;
      this.addModalShown=true;
    },
    // this is the callback function used by the add modal to actually send the api command to add the bookmark
    addBookmark: function(bookmark,loadingComponent) {
      HTTP.post('api/bookmarks',bookmark,{timeout: 5000}).then((response) => {
        this.$toast.open({
          message: 'Bookmark \"'+bookmark.name+'\" has been added.',
          type: 'is-success',
          position: 'is-bottom'
        });

        // also here push the bookmark to the bookmarks array and don't send another request to the ESP
        this.bookmarks.push(bookmark);

        this.activeBookmark=undefined;
        this.addModalShown=false;
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
  // once the compenent is created, get the available bookmarks from the ESP
  created: function() {
    HTTP.get('/api/bookmarks').then((response) => {
      if (response.status == 200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json"))) {
        this.bookmarks=response.data;
      } else {
        this.bookmarks = [];
      }
    }).catch((e) => {
      this.bookmarks = [];
      this.$toast.open({
        message: 'Couldn\'t get bookmarks from ESP.',
        type: 'is-danger',
        position: 'is-bottom'
      });
    })
  }
}
</script>

<style scoped>

</style>
