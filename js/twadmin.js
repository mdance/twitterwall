$(document).ready(function(){
  $('form#twitterwall-adminpage input#edit-selectall').click(function(){
    var status = this.checked;
    var checkboxes = $('form#twitterwall-adminpage tbody .form-checkbox');
    
    checkboxes.each(function(){
      this.checked = status;
    });
  });
});