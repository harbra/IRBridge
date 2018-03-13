<!--
This vue.js file contains a modal that can be used to create and modify names.
-->
<template>
<form v-on:submit.prevent="commitModal()">
<div class="card">
  <div class="card-header">
    <div class="card-header-title">
      {{dialogTitle}}
    </div>
  </div>
  <div class="card-content">
    <div class="field">
      <label class="label">Name</label>
      <p class="control">
        <input class="input" type="text" placeholder="Bookmark name" required v-model="myname" v-bind:class="{'is-danger': iserrorname}">
      </p>
    </div>

    <div class="card-footer">
      <a class="card-footer-item button is-primary" v-on:click="commitModal()">
        <span class="icon">
          <i class="icon-save"></i>
        </span>
        <span>Save</span>
      </a>
      <a class="card-footer-item button is-danger" v-on:click="cancelModal()">
        <span class="icon">
          <i class="icon-remove"></i>
        </span>
        <span>Cancel</span>
      </a>
    </div>
  </div>
</div>
</form>
</template>

<script>
import {
  HTTP
} from '../plugins/axios'

export default {
  name: 'ModalName',
  data() {
    return {
      myname: this.name,
      iserrorname: false,
    }
  },
  props: {
    dialogTitle: { type: String,
                   required: true },
    name: String
  },
  methods: {
    validate: function() {
      if(this.myname=="" || this.myname==undefined || this.myname==null) {
        this.iserrorname=true;
        this.$toast.open({
                      message: 'Please enter a name.',
                      type: 'is-warning',
                      position: 'is-top'
                  });
        return false;
      } else {
        this.iserrorname=false;
      }

      return true;
    },
    cancelModal: function() {
      this.$emit('cancel');
    },
    commitModal: function() {
      if(!this.validate()) {
        return;
      }

      const loadingComponent = this.$loading.open();

      var retval={
          name: this.myname
        }

      this.$emit('commit',retval,loadingComponent);
    }
  }
}
</script>
