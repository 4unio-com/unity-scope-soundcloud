package main

import (
	"encoding/json"
	"net/http"
	"net/url"
	"log"

	"launchpad.net/go-unityscopes/v1"
	"launchpad.net/go-onlineaccounts/v1"
)

const providerIcon = "/usr/share/icons/unity-icon-theme/places/svg/service-soundcloud.svg"

const searchCategoryTemplate = `{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "small"
  },
  "components": {
    "title": "title",
    "art":  "art",
    "subtitle": "username"
  }
}`

const loginNagTemplate = `{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-total_results": "large",
    "card-background": "color:///#DD4814"
  },
  "components": {
    "title": "title",
    "background": "background",
    "art" : {
      "aspect-ratio": 100.0
    }
  }}
`

type SoundCloudScope struct {
	BaseURI     string
	ClientId    string
	Accounts    *accounts.Watcher
}

type trackInfo struct {
	Title  string `json:"title"`
	Length int    `json:"length"`
	Source string `json:"source"`
}

type actionInfo struct {
	Id    string `json:"id"`
	Label string `json:"label"`
	Icon  string `json:"icon,omitempty"`
	Uri   string `json:"uri,omitempty"`
}

type user struct {
	Id        int    `json:"id"`
	Username  string `json:"username"`
	Uri       string `json:"uri"`
	AvatarUrl string `json:"avatar_url"`
}

type track struct {
	Id           int    `json:"id"`
	CreatedAt    string `json:"created_at"`
	User         user   `json:"user"`
	Streamable   bool   `json:"streamable"`
	Downloadable bool   `json:"downloadable"`

	PermalinkUrl string `json:"permalink_url"`
	PurchaseUrl  string `json:"purchase_url"`
	ArtworkUrl   string `json:"artwork_url"`
	StreamUrl    string `json:"stream_url"`
	DownloadUrl  string `json:"download_url"`
	VideoUrl     string `json:"video_url"`

	Title        string `json:"title"`
	Description  string `json:"description"`
	LabelName    string `json:"label_name"`
	Duration     int    `json:"duration"`
	License      string `json:"license"`
}

type AccountLoginAction int

const (
        _ = AccountLoginAction(iota)
        DoNothing
        InvalidateResults
        ContinueActivation
)

type accountDetails struct {
        ServiceName string `json:"service_name"`
        ServiceType string `json:"service_type"`
        ProviderName string `json:"provider_name"`
        LoginPassedAction AccountLoginAction `json:"login_passed_action"`
        LoginFailedAction AccountLoginAction `json:"login_failed_action"`
}

func (sc *SoundCloudScope) buildUrl(resource string, params map[string]string) string {
	query := make(url.Values)
	query.Set("client_id", sc.ClientId)
	for key, value := range params {
		query.Set(key, value)
	}
	return sc.BaseURI + resource + ".json?" + query.Encode()
}

func (sc *SoundCloudScope) get(resource string, params map[string]string, result interface{}) error {
	resp, err := http.Get(sc.buildUrl(resource, params))
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	decoder := json.NewDecoder(resp.Body)
	return decoder.Decode(result)
}

func (sc *SoundCloudScope) SetScopeBase(base *scopes.ScopeBase) {
	log.Println("Scope base is", base)
	var s interface{}
	base.Settings(&s)
	log.Println("Settings:", s)
}

func (sc *SoundCloudScope) Search(q *scopes.CannedQuery, metadata *scopes.SearchMetadata, reply *scopes.SearchReply, cancelled <-chan bool) error {
	// We currently don't have any surfacing results
	var tracks []track
	query := q.QueryString()
	if query == "" {
		accessToken := ""
		services := sc.Accounts.EnabledServices()
		if len(services) > 0 {
			service := services[0]
			// If the service is in an error state, try
			// and refresh it.
			if service.Error != nil {
				service = sc.Accounts.Refresh(service.AccountId, false)
			}
			if service.Error != nil {
				accessToken = service.AccessToken
			}
		}

		if accessToken != "" {
			if err := sc.get("/me/favorites", map[string]string{"limit": "30", "order": "hotness", "oauth_token": accessToken}, &tracks); err != nil {
				return err
			}
		} else {
			// Not logged in, so add nag
			cat := reply.RegisterCategory("nag", "", "", loginNagTemplate)
			result := scopes.NewCategorisedResult(cat)
			result.SetURI(q.ToURI())
			result.SetTitle("Log in to SoundCloud")
			result.Set("online_account_details", accountDetails{
				ServiceName: "soundcloud",
				ServiceType: "sharing",
				ProviderName: "soundcloud",
				LoginPassedAction: InvalidateResults,
				LoginFailedAction: DoNothing,
			})
			if err := reply.Push(result); err != nil {
				return err
			}

			if err := sc.get("/tracks", map[string]string{"limit": "30", "order": "hotness"}, &tracks); err != nil {
				return err
			}
		}
	} else {
		if err := sc.get("/tracks", map[string]string{"q": query, "limit": "30", "order": "hotness"}, &tracks); err != nil {
			return err
		}
	}

	cat := reply.RegisterCategory("soundcloud", "SoundCloud", "", searchCategoryTemplate)
	for _, track := range tracks {
		result := scopes.NewCategorisedResult(cat)
		result.SetURI(track.PermalinkUrl)
		result.SetTitle(track.Title)
		if track.ArtworkUrl != "" {
			result.SetArt(track.ArtworkUrl)
		} else {
			result.SetArt(track.User.AvatarUrl)
		}
		result.Set("duration", track.Duration)
		result.Set("username", track.User.Username)
		result.Set("label", track.LabelName)
		result.Set("description", track.Description)
		result.Set("stream-url", track.StreamUrl)
		result.Set("purchase-url", track.PurchaseUrl)
		result.Set("video-url", track.VideoUrl)
		if err := reply.Push(result); err != nil {
			return err
		}
	}
	return nil
}

func (sc *SoundCloudScope) Preview(result *scopes.Result, metadata *scopes.ActionMetadata, reply *scopes.PreviewReply, cancelled <-chan bool) error {
	header := scopes.NewPreviewWidget("header", "header")
	header.AddAttributeMapping("title", "title")
	header.AddAttributeMapping("subtitle", "username")

	art := scopes.NewPreviewWidget("art", "image")
	art.AddAttributeMapping("source", "art")

	var (
		title, streamUrl string
		duration int
	)
	if err := result.Get("title", &title); err != nil {
		return err
	}
	if err := result.Get("duration", &duration); err != nil {
		return err
	}
	if err := result.Get("stream-url", &streamUrl); err != nil {
		return err
	}
	tracks := scopes.NewPreviewWidget("tracks", "audio")
	tracks.AddAttributeValue("tracks", []trackInfo{trackInfo{
		Title: title,
		Length: duration / 1000,
		Source: streamUrl + "?client_id=" + sc.ClientId,
	}})

	buttons := []actionInfo{
		actionInfo{Id: "play", Label: "Play", Icon: providerIcon},
	}
	var purchaseUrl string
	if err := result.Get("purchase-url", &purchaseUrl); err == nil && purchaseUrl != "" {
		buttons = append(buttons, actionInfo{Id: "buy", Label: "Buy", Uri: purchaseUrl})
	}
	var videoUrl string
	if err := result.Get("video-url", &videoUrl); err == nil && videoUrl != "" {
		buttons = append(buttons, actionInfo{Id: "video", Label: "Watch video", Uri: videoUrl})
	}

	actions := scopes.NewPreviewWidget("actions", "actions")
	actions.AddAttributeValue("actions", buttons)

	description := scopes.NewPreviewWidget("description", "text")
	description.AddAttributeMapping("text", "description")

	return reply.PushWidgets(header, art, tracks, actions, description)
}

func main() {
	log.Println("Setting up accounts")
	watcher := accounts.NewWatcher("sharing", []string{"soundcloud"})
	watcher.Settle()
	log.Println("Starting soundcloud scope")
	scope := &SoundCloudScope{
		BaseURI: "https://api.soundcloud.com",
		ClientId: "eadbbc8380aa72be1412e2abe5f8e4ca",
		Accounts: watcher,
	}

	if err := scopes.Run(scope); err != nil {
		log.Fatalln(err)
	}
}
